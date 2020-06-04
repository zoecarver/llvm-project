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

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
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
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/Local.h"

using namespace llvm;

#define DEBUG_TYPE "smart-ptr-lifetime-opts"

STATISTIC(NumUniquePtrDestructorsRemoved,
          "Number of unique_ptr destructors that were able to be removed");

//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

namespace {

/// The "Lifetime" state information for memory.
enum class Lifetime {
  Unknown, // Unkown may be "Live"
  MovedTo,
  Empty // Empty means it was either "MovedFrom" or "Deleted".
};

/// Various known functions we will use to classify call instructions.
enum class KnownFunc { Unknown, MoveConst, Destructor };

/// Classify a function based on the functions \p Name.
static KnownFunc lookupFunc(StringRef Name) {
  // Fastpath: because we call lookupFunc on every CallInst. Make sure there's
  // a semantics tag before we make any other lookups.
  if (!Name.contains("__SEMANTICS"))
    return KnownFunc::Unknown;
  if (Name.contains("__SEMANTICS_unique_ptr_move"))
    return KnownFunc::MoveConst;
  if (Name.contains("__SEMANTICS_unique_ptr_destroy"))
    return KnownFunc::Destructor;
  return KnownFunc::Unknown;
}

/// Set the lifetime of the unique_ptr move constructor arguments. The arguments
/// are from the move constructor, \p Call. The lifetime information is stored
/// in \p LifetimeLookup.
///
/// The "To" argument (first) is "moved to" meaning it is live but also, if the
/// next use of the value "To" is a call, we know that call will call the
/// destructor.
///
/// The "From" argument (second) is "empty" meaning the pointer is a nullptr. In
/// this case, the following destructor will be a noop so we can remove it.
/// TODO: in the future, we can null-out the unique_ptr here which may help
/// later optimization passes.
static void mapMoveConstArgs(CallInst *Call,
                             DenseMap<Value *, Lifetime> &LifetimeLookup) {
  Value *To = Call->getArgOperand(0);
  Value *From = Call->getArgOperand(1);
  LifetimeLookup[To] = Lifetime::MovedTo;
  // TODO: this is ususally passed through `std::move` so we should check if we
  // can trace this back and further.
  LifetimeLookup[From] = Lifetime::Empty;
}

/// Try to determin if we know the destructor is going to be a noop. The
/// destructor is \p Call. The lifetime information is stored in \p
/// LifetimeLookup.
///
/// If the lifetime of the pointer is known, and empty, it return true (the
/// destructor is noop). Otherwise, set the lifetime "Empty" because after this
/// call, the unique_ptr will hold a nullptr and return false.
static bool isDestructorNoop(CallInst *Call,
                             DenseMap<Value *, Lifetime> &LifetimeLookup) {
  Value *UniquePtr = Call->getArgOperand(0);
  Lifetime &UniquePtrLifetime = LifetimeLookup[UniquePtr];
  // If we knwo the unique_ptr holds nullptr, the destructor is a noop.
  if (UniquePtrLifetime == Lifetime::Empty)
    return true;
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
    SmallVector<Instruction *, 8> DeadInsts;
    DenseMap<Value *, Lifetime> LifetimeLookup;
    
    auto InvalidateUnkown = [&](Instruction &Inst) {
      // If there are any other uses of tracked values, those are unknown so,
      // assume they make the pointer live (or "Unkown").
      for (size_t OpIdx = 0; OpIdx < Inst.getNumOperands(); ++OpIdx) {
        Value *Op = Inst.getOperand(OpIdx);
        auto FoundItr = llvm::find_if(LifetimeLookup, [Op](auto Both) {
          return Both.getFirst() == Op;
        });
        if (FoundItr != LifetimeLookup.end())
          FoundItr->getSecond() = Lifetime::Unknown;
      }
    };
    
    for (BasicBlock &Block : F) {
      for (Instruction &Inst : Block) {
        // If this is a call instruction, see if it gives us any lifetime
        // information.
        if (CallInst *Call = dyn_cast<CallInst>(&Inst)) {
          // If we can't find the called function, invalidate any operands and
          // skip the call.
          if (!Call->getCalledFunction()) {
            InvalidateUnkown(Inst);
            continue;
          }
          StringRef Name = Call->getCalledFunction()->getName();
          KnownFunc Func = lookupFunc(Name);
          if (Func == KnownFunc::MoveConst) {
            mapMoveConstArgs(Call, LifetimeLookup);
            // TODO: We can lower the move constructor, replacing it with a
            // store.
            continue;
          } else if (Func == KnownFunc::Destructor) {
            if (isDestructorNoop(Call, LifetimeLookup)) {
              DeadInsts.push_back(Call);
              ++NumUniquePtrDestructorsRemoved;
            }
            continue;
          }
          // If this call uses a value we're keeping track of.
          for (size_t ArgIdx = 0; ArgIdx < Call->getNumArgOperands();
               ++ArgIdx) {
            Value *Arg = Call->getArgOperand(ArgIdx);
            auto FoundItr = llvm::find_if(LifetimeLookup, [Arg](auto Both) {
              return Both.getFirst() == Arg;
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
          // Nothing more to be done with this instruction; continue.
          continue;
        }
        // If the instruction is unkown, invalidate any operands that we are
        // tracking.
        InvalidateUnkown(Inst);
      }
      // We're in a new block, set all tracked values to "Unknown" because we
      // don't know what block we may have come from.
      // TODO: in the future we may be able to dominance analysis to safely
      // perform multi-block optimizations.
      for (auto &Both : LifetimeLookup)
        Both.getSecond() = Lifetime::Unknown;
    }

    Changed = !DeadInsts.empty();
    for (Instruction *I : DeadInsts)
      I->eraseFromParent();

    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {}
};

} // end anonymous namespace

char SmartPtrLifetime::ID = 0;

INITIALIZE_PASS(SmartPtrLifetime, "smart-ptr-lifetime-opts",
                "Smart Pointer Lifetime Optimizations", false, false)

FunctionPass *llvm::createSmartPtrLifetimePass() {
  return new SmartPtrLifetime();
}
