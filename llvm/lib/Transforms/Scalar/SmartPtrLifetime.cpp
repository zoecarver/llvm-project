//===- DeadStoreElimination.cpp - Fast Dead Store Elimination -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements basic smart pointer lifetime optimizations.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/DeadStoreElimination.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/MemorySSAUpdater.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/DebugCounter.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/AssumeBundleBuilder.h"
#include "llvm/Transforms/Utils/Local.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <utility>

using namespace llvm;

#define DEBUG_TYPE "smart-ptr-lifetime-opts"

STATISTIC(NumUniquePtrDestructorsRemoved, "Number of unique_ptr destructors that were able to be removed");


//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

namespace {

enum class Lifetime {
  Unknown, // Unkown may be "Live"
  MovedTo,
  Empty // Empty means it was either "MovedFrom" or "Deleted".
};

enum class KnownFunc {
  Unknown,
  MoveConst,
  Destructor
};

static KnownFunc LookupFunc(StringRef Name) {
  return StringSwitch<KnownFunc>(Name)
    .Case("_ZN6my_ptrIiEC1EOS0_", KnownFunc::MoveConst)
    .Case("_ZN6my_ptrIiED1Ev", KnownFunc::Destructor)
    .Default(KnownFunc::Unknown);
}

static void mapAndLowerMoveConst(CallInst *Call,
                                 DenseMap<Value*, Lifetime> &LifetimeLookup) {
  Value *To = Call->getArgOperand(0);
  Value *From = Call->getArgOperand(1);
  LifetimeLookup[To] = Lifetime::MovedTo;
  LifetimeLookup[From] = Lifetime::Empty;
  // TODO: lower this.
}

static bool handleDestructor(CallInst *Call,
                             DenseMap<Value*, Lifetime> &LifetimeLookup) {
  Value *UniquePtr = Call->getArgOperand(0);
  Lifetime &UniquePtrLifetime = LifetimeLookup[UniquePtr];
  // If we knwo the unique_ptr holds nullptr, the destructor is a noop.
  if (UniquePtrLifetime == Lifetime::Empty) {
    // TODO: assert no results.
    return true;
  }
  // Otherwise, mark it as dead.
  UniquePtrLifetime = Lifetime::Empty;
  return false;
}

/// A legacy pass for the legacy pass manager that wraps \c DSEPass.
class SmartPtrLifetime : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  SmartPtrLifetime() : FunctionPass(ID) {
    initializeSmartPtrLifetimePass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    bool Changed = false;
    SmallVector<Instruction*, 8> DeadInsts;
    DenseMap<Value*, Lifetime> LifetimeLookup;
    for (BasicBlock &Block : F) {
      for (Instruction &Inst : Block) {
        if (CallInst *Call = dyn_cast<CallInst>(&Inst)) {
          StringRef Name = Call->getCalledFunction()->getName();
          KnownFunc Func = LookupFunc(Name);
          if (Func == KnownFunc::MoveConst) {
            mapAndLowerMoveConst(Call, LifetimeLookup);
            continue;
          } else if (Func == KnownFunc::Destructor) {
            if (handleDestructor(Call, LifetimeLookup))
              DeadInsts.push_back(Call);
            continue;
          }
          // If this call uses a value we're keeping track of.
          for (size_t ArgIdx = 0; ArgIdx < Call->getNumArgOperands(); ++ArgIdx) {
            Value *Arg = Call->getArgOperand(ArgIdx);
            auto FoundItr = llvm::find_if(LifetimeLookup, [Arg](auto both) {
              return both.getFirst() == Arg;
            });
            if (FoundItr == LifetimeLookup.end())
              continue;
            Lifetime &TrackedLifetime = FoundItr->getSecond();
            // If we moved into this value and then passed it as an arg, we
            // know that it will be deleted in the context it was passed into.
            if (TrackedLifetime == Lifetime::MovedTo)
              TrackedLifetime = Lifetime::Empty;
            else
              // Otherwise, we have no idea what might happen to the operand.
              TrackedLifetime = Lifetime::Unknown;
          }
        }
        // TODO: find if any operands are in lifetime lookup and invalidate them.
      }
    }
    
    Changed = !DeadInsts.empty();
    for (Instruction *I : DeadInsts) {
      llvm::errs() << "Killed: ";
      I->dump();
      I->eraseFromParent();
    }
    
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    
  }
};

} // end anonymous namespace

char SmartPtrLifetime::ID = 0;

INITIALIZE_PASS(SmartPtrLifetime, "smart-ptr-lifetime-opts",
                "Smart Pointer Lifetime Optimizations", false, false)

FunctionPass *llvm::createSmartPtrLifetimePass() {
  return new SmartPtrLifetime();
}
