// LLVM for c-like language w/ fn:
//

#include "gep.h"
#include "util.h"

//--- GEP ---
// https://llvm.org/docs/GetElementPtr.html, https://llvm.org/docs/LangRef.html#getelementptr-instruction
// - doesn't access memory, only computes
// - args
//   - 1st is type for use as basis for calculation
//   - 2nd pointer or vector<pointer> and is base address to start from
//   - remaining are indices which which of the elements of the aggregate are indexed
//     - interpretation of each index dependent on type being indexed into
//     - 1st index always indexes the the 2nd arg
//     - 2nd index indexes a value of the type pointed to
//     - 1st type index must be pointer value, subsequent types can be arrays, vectors, and structs (but not pointers)
//   - 
//   - 
//   - 
//   - 
//   - 
//-----------

//--------------------- LLVM C API ---------------------
//  LLVMValueRef LLVMConstGEP2         (LLVMTypeRef Ty, LLVMValueRef ConstantVal, LLVMValueRef *ConstantIndices, unsigned NumIndices);
//  LLVMValueRef LLVMConstInBoundsGEP2 (LLVMTypeRef Ty, LLVMValueRef ConstantVal, LLVMValueRef *ConstantIndices, unsigned NumIndices);
//
//  LLVMValueRef LLVMBuildGEP2         (LLVMBuilderRef B, LLVMTypeRef Ty, LLVMValueRef Pointer, LLVMValueRef *Indices, unsigned NumIndices, const char *Name);
//  LLVMValueRef LLVMBuildInBoundsGEP2 (LLVMBuilderRef B, LLVMTypeRef Ty, LLVMValueRef Pointer, LLVMValueRef *Indices, unsigned NumIndices, const char *Name);
//  LLVMValueRef LLVMBuildStructGEP2   (LLVMBuilderRef B, LLVMTypeRef Ty, LLVMValueRef Pointer, unsigned Idx, const char *Name);
//-----------------------------------------------------

LLVMValueRef create_get_snd_int_fn (
  LLVMContextRef ctx,
  LLVMModuleRef mod,
  const char* name,
  int num_bits
)
{
  // Types
  LLVMTypeRef int64_type   = LLVMInt64TypeInContext(ctx);
  LLVMTypeRef int_type     = LLVMIntTypeInContext(ctx, num_bits);
  LLVMTypeRef int_ptr_type = LLVMPointerType(int_type, 0 /* AddressSpace */);

  // New fn: get_int (Int*) Int
  unsigned num_params       = 1;
  LLVMTypeRef param_types[] = { int_ptr_type };
  LLVMTypeRef return_type   = int_type;
  LLVMTypeRef signature     = LLVMFunctionType(return_type, param_types, num_params, F);
  LLVMValueRef fn           = LLVMAddFunction(mod, name, signature);

  // Get args
  LLVMValueRef arg_int_ptr = LLVMGetParam(fn, 0);

  // Basic blocks
  LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(ctx, fn, "entry");

  // Position builder to start where we left off in 'entry' block
  LLVMBuilderRef builder = LLVMCreateBuilderInContext(ctx);
  LLVMPositionBuilderAtEnd(builder, entry);

  // Compute position in GEP
  LLVMValueRef one      = LLVMConstInt(int64_type, 1, T /* sign extended */);
  LLVMValueRef* indexes = &one;
  int num_indexes       = 1;

  //LLVMValueRef snd_int_ptr = LLVMBuildGEP2(builder, int_type, arg_int_ptr, indexes, num_indexes, "");
  LLVMValueRef snd_int_ptr = LLVMBuildInBoundsGEP2(builder, int_type, arg_int_ptr, indexes, num_indexes, "");
  LLVMValueRef snd_int     = LLVMBuildLoad2(builder, int_type, snd_int_ptr, "");

  // Return
  LLVMBuildRet(builder, snd_int);

  // Cleanup
  LLVMDisposeBuilder(builder);

  return snd_int;
}

//    struct munger_struct {
//      int f1;
//      int f2;
//    };
//
//    void munge(struct munger_struct *P) {
//      P[0].f1 = P[1].f1 + P[2].f2;
//    }
//    ...
//    struct munger_struct Array[3];
//    ...
//    munge(Array);
//
//
//    define void @munge(%struct.munger_struct* %P){
//      entry:
//        %tmp = getelementptr %struct.munger_struct, %struct.munger_struct* %P, i32 1, i32 0
//        %tmp1 = load i32, i32* %tmp
//        %tmp2 = getelementptr %struct.munger_struct, %struct.munger_struct* %P, i32 2, i32 1
//        %tmp3 = load i32, i32* %tmp2
//        %tmp4 = add i32 %tmp3, %tmp1
//        %tmp5 = getelementptr %struct.munger_struct, %struct.munger_struct* %P, i32 0, i32 0
//        store i32 %tmp4, i32* %tmp5
//        ret void
//    }
LLVMValueRef create_munge_fn (
  LLVMContextRef ctx,
  LLVMModuleRef mod,
  const char* name,
  int num_bits
)
{
  // Types
  LLVMTypeRef int32_type = LLVMInt32TypeInContext(ctx);
  LLVMTypeRef int64_type = LLVMInt64TypeInContext(ctx);
  LLVMTypeRef int_type   = LLVMIntTypeInContext(ctx, num_bits);

  LLVMTypeRef munger_struct_elem_types[] = { int_type, int_type };
  LLVMTypeRef munger_struct_type         = LLVMStructTypeInContext(ctx, munger_struct_elem_types, 2, F /* Packed */);
  LLVMTypeRef munger_struct_ptr_type     = LLVMPointerType(munger_struct_type, 0 /* AddressSpace */);

  // New fn: get_int (Int*) Int
  unsigned num_params       = 1;
  LLVMTypeRef param_types[] = { munger_struct_ptr_type };
  LLVMTypeRef return_type   = LLVMVoidTypeInContext(ctx);
  LLVMTypeRef signature     = LLVMFunctionType(return_type, param_types, num_params, F);
  LLVMValueRef fn           = LLVMAddFunction(mod, name, signature);

  // Get args
  LLVMValueRef arg_munger_struct_ptr = LLVMGetParam(fn, 0);

  // Consts
  LLVMValueRef zero = LLVMConstInt(int32_type, 0, T /* sign extended */);
  LLVMValueRef one  = LLVMConstInt(int32_type, 1, T /* sign extended */);
  LLVMValueRef two  = LLVMConstInt(int32_type, 2, T /* sign extended */);

  // Basic blocks
  LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(ctx, fn, "entry");

  // Position builder to start where we left off in 'entry' block
  LLVMBuilderRef builder = LLVMCreateBuilderInContext(ctx);
  LLVMPositionBuilderAtEnd(builder, entry);

  // Compute position in GEP
//        %tmp = getelementptr %struct.munger_struct, %struct.munger_struct* %P, i32 1, i32 0
//        %tmp1 = load i32, i32* %tmp
//        %tmp2 = getelementptr %struct.munger_struct, %struct.munger_struct* %P, i32 2, i32 1
//        %tmp3 = load i32, i32* %tmp2
//        %tmp4 = add i32 %tmp3, %tmp1
//        %tmp5 = getelementptr %struct.munger_struct, %struct.munger_struct* %P, i32 0, i32 0
//        store i32 %tmp4, i32* %tmp5
  
  // Get P[1].f1
  //      LLVMValueRef x_addr = LLVMBuildGEP2(builder, dbl_type, arg_ptr_x, indexes, num_indexes, "");
  //      LLVMValueRef x_i    = LLVMBuildLoad2(builder, dbl_type, x_addr, "");
  LLVMValueRef elem_1_f1 = NULL;
  {
    LLVMValueRef indexes[] = { one, zero };
    int num_indexes        = 2;
    LLVMValueRef ptr       = LLVMBuildGEP2(builder, munger_struct_type, arg_munger_struct_ptr, indexes, num_indexes, "");

    elem_1_f1 = LLVMBuildLoad2(builder, int_type, ptr, "");
  }

  // Get P[2].f2
  LLVMValueRef elem_2_f2 = NULL;
  {
    LLVMValueRef indexes[] = { two, one };
    int num_indexes        = 2;
    LLVMValueRef ptr       = LLVMBuildGEP2(builder, munger_struct_type, arg_munger_struct_ptr, indexes, num_indexes, "");

    elem_2_f2 = LLVMBuildLoad2(builder, int_type, ptr, "");
  }

  // P[1].f1 + P[2].f2
  LLVMValueRef tmp = LLVMBuildAdd(builder, elem_1_f1, elem_2_f2, "tmp");

  // P[0].f1 = P[1].f1 + P[2].f2;
  {
    LLVMValueRef indexes[] = { zero, zero };
    int num_indexes        = 2;
    LLVMValueRef ptr       = LLVMBuildGEP2(builder, munger_struct_type, arg_munger_struct_ptr, indexes, num_indexes, "");

    LLVMBuildStore(builder, tmp, ptr);
  }

  // Return
  LLVMBuildRetVoid(builder);

  // Cleanup
  LLVMDisposeBuilder(builder);

  return fn;
}

// http://releases.llvm.org/9.0.0/docs/LangRef.html#getelementptr-instruction
//
//    struct RT {
//      char A;
//      int B[10][20];
//      char C;
//    };
//    struct ST {
//      int X;
//      double Y;
//      struct RT Z;
//    };
//    
//    int *foo(struct ST *s) {
//      return &s[1].Z.B[5][13];
//    }
