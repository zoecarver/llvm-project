; RUN: opt < %s -smart-ptr-lifetime-opts -S | FileCheck %s

target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.15.0"

%struct.unique_ptr = type { i32* }

declare noalias nonnull i8* @new(i64)

declare void @init(%struct.unique_ptr*, i32*)

declare i32* @get_ptr(%struct.unique_ptr*)

declare void @guaranteed(%struct.unique_ptr* )

declare  %struct.unique_ptr* @__SEMANTICS_unique_ptr_global_move(%struct.unique_ptr*)

declare void @__SEMANTICS_unique_ptr_move_const(%struct.unique_ptr*, %struct.unique_ptr*)

declare void @owner(%struct.unique_ptr* %agg.tmp)

declare void @__SEMANTICS_unique_ptr_destroy(%struct.unique_ptr*)

; CHECK-LABEL: @test_gauranteed_and_owned
; CHECK: [[P1:%.*]] = alloca %struct.unique_ptr
; CHECK: [[P_TMP:%.*]] = alloca %struct.unique_ptr
; CHECK: [[NEW_MEM:%.*]] = call noalias nonnull i8* @new
; CHECK: [[I_PTR:%.*]] = bitcast i8* [[NEW_MEM]] to i32*
; CHECK: call void @init(%struct.unique_ptr* [[P1]], i32* [[I_PTR]])
; CHECK: [[RAW_PTR:%.*]] = call i32* @get_ptr(%struct.unique_ptr* [[P1]])
; CHECK: store i32 42, i32* [[RAW_PTR]]
; CHECK: call void @guaranteed(%struct.unique_ptr* [[P1]])
; CHECK: [[MOVED:%.*]] = call %struct.unique_ptr* @__SEMANTICS_unique_ptr_global_move(%struct.unique_ptr* [[P1]])
; CHECK: call void @__SEMANTICS_unique_ptr_move_const(%struct.unique_ptr* [[P_TMP]], %struct.unique_ptr* [[MOVED]])
; CHECK: call void @owner(%struct.unique_ptr* [[P_TMP]])
; Check that there are no calls to "__SEMANTICS_unique_ptr_destroy."
; CHECK-NEXT: ret void
define void @test_gauranteed_and_owned() {
entry:
  %p = alloca %struct.unique_ptr, align 8
  %agg.tmp = alloca %struct.unique_ptr, align 8
  %call = call noalias nonnull i8* @new(i64 16)
  %0 = bitcast i8* %call to i32*
  call void @init(%struct.unique_ptr* %p, i32* %0)
  %call1 = call i32* @get_ptr(%struct.unique_ptr* %p)
  store i32 42, i32* %call1, align 4
  call void @guaranteed(%struct.unique_ptr*  %p)
  %call2 = call  %struct.unique_ptr* @__SEMANTICS_unique_ptr_global_move(%struct.unique_ptr*  %p)
  call void @__SEMANTICS_unique_ptr_move_const(%struct.unique_ptr* %agg.tmp, %struct.unique_ptr*  %call2)
  call void @owner(%struct.unique_ptr* %agg.tmp)
  call void @__SEMANTICS_unique_ptr_destroy(%struct.unique_ptr* %agg.tmp)
  call void @__SEMANTICS_unique_ptr_destroy(%struct.unique_ptr* %p)
  ret void
}

; CHECK-LABEL: @test_owned_then_guaranteed
; CHECK: [[P1:%.*]] = alloca %struct.unique_ptr
; CHECK: [[P_TMP:%.*]] = alloca %struct.unique_ptr
; CHECK: [[NEW_MEM:%.*]] = call noalias nonnull i8* @new
; CHECK: [[I_PTR:%.*]] = bitcast i8* [[NEW_MEM]] to i32*
; CHECK: call void @init(%struct.unique_ptr* [[P1]], i32* [[I_PTR]])
; CHECK: [[RAW_PTR:%.*]] = call i32* @get_ptr(%struct.unique_ptr* [[P1]])
; CHECK: store i32 42, i32* [[RAW_PTR]]
; CHECK: [[MOVED:%.*]] = call %struct.unique_ptr* @__SEMANTICS_unique_ptr_global_move(%struct.unique_ptr* [[P1]])
; CHECK: call void @__SEMANTICS_unique_ptr_move_const(%struct.unique_ptr* [[P_TMP]], %struct.unique_ptr* [[MOVED]])
; CHECK: call void @owner(%struct.unique_ptr* [[P_TMP]])
; CHECK-NEXT: call void @guaranteed(%struct.unique_ptr* [[P1]])
; CHECK-NEXT: call void @__SEMANTICS_unique_ptr_destroy(%struct.unique_ptr* [[P1]])
; CHECK-NEXT: ret void
define void @test_owned_then_guaranteed() {
entry:
  %p = alloca %struct.unique_ptr, align 8
  %agg.tmp = alloca %struct.unique_ptr, align 8
  %call = call noalias nonnull i8* @new(i64 16)
  %0 = bitcast i8* %call to i32*
  call void @init(%struct.unique_ptr* %p, i32* %0)
  %call1 = call i32* @get_ptr(%struct.unique_ptr* %p)
  store i32 42, i32* %call1, align 4
  %call2 = call  %struct.unique_ptr* @__SEMANTICS_unique_ptr_global_move(%struct.unique_ptr*  %p)
  call void @__SEMANTICS_unique_ptr_move_const(%struct.unique_ptr* %agg.tmp, %struct.unique_ptr*  %call2)
  call void @owner(%struct.unique_ptr* %agg.tmp)
  ; This function is unkown, it could call reset or release so, we can't optimize p's destructor.
  call void @guaranteed(%struct.unique_ptr*  %p)
  ; We can remove this destructor, though.
  call void @__SEMANTICS_unique_ptr_destroy(%struct.unique_ptr* %agg.tmp)
  call void @__SEMANTICS_unique_ptr_destroy(%struct.unique_ptr* %p)
  ret void
}

; We can't yet optimize across multiple blocks.
; CHECK-LABEL: @test_multiblock
; CHECK: next:
; CHECK-NEXT: __SEMANTICS_unique_ptr_destroy
; CHECK-NEXT: __SEMANTICS_unique_ptr_destroy
; CHECK-NEXT: ret void
define void @test_multiblock() {
entry:
  %p = alloca %struct.unique_ptr, align 8
  %agg.tmp = alloca %struct.unique_ptr, align 8
  %call = call noalias nonnull i8* @new(i64 16)
  %0 = bitcast i8* %call to i32*
  call void @init(%struct.unique_ptr* %p, i32* %0)
  %call1 = call i32* @get_ptr(%struct.unique_ptr* %p)
  store i32 42, i32* %call1, align 4
  %call2 = call  %struct.unique_ptr* @__SEMANTICS_unique_ptr_global_move(%struct.unique_ptr*  %p)
  call void @__SEMANTICS_unique_ptr_move_const(%struct.unique_ptr* %agg.tmp, %struct.unique_ptr*  %call2)
  call void @owner(%struct.unique_ptr* %agg.tmp)
  br label %next
  
next:
  call void @__SEMANTICS_unique_ptr_destroy(%struct.unique_ptr* %agg.tmp)
  call void @__SEMANTICS_unique_ptr_destroy(%struct.unique_ptr* %p)
  ret void
}
