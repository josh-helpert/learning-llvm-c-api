// LLVM IR for c-like language w/ fn:
//
//   int fib (int x)
//   {
//     if (x <= 2) return 1;
//     return fib(x - 1) + fib(x - 2);
//   }

#include "fib.h"
#include "../util.h"

LLVMValueRef create_fib_fn (
  LLVMContextRef ctx,
  LLVMModuleRef mod,
  const char* name,
  unsigned num_bits
)
{
  // Build fn
  LLVMTypeRef param_types[] = { LLVMIntTypeInContext(ctx, num_bits) };
  LLVMTypeRef return_type   = LLVMIntTypeInContext(ctx, num_bits);
  LLVMTypeRef signature     = LLVMFunctionType(return_type, param_types, 1, F);
  LLVMValueRef fn           = LLVMAddFunction(mod, name, signature);

  LLVMSetLinkage(fn, LLVMExternalLinkage);

  //
  LLVMValueRef one = LLVMConstInt(LLVMIntTypeInContext(ctx, num_bits), 1, F);
  LLVMValueRef two = LLVMConstInt(LLVMIntTypeInContext(ctx, num_bits), 2, F);
  
  LLVMValueRef x = LLVMGetParam(fn, 0);
  //x->set_name    = "x";

  // Create blocks
  LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(ctx, fn, "entry");
  LLVMBasicBlockRef ret   = LLVMAppendBasicBlockInContext(ctx, fn, "ret");
  LLVMBasicBlockRef recur = LLVMAppendBasicBlockInContext(ctx, fn, "recur");

  // Create and position builder
  LLVMBuilderRef builder = LLVMCreateBuilderInContext(ctx);
  LLVMPositionBuilderAtEnd(builder, entry);

  // Create `if (x <= 2) goto ret`
  LLVMValueRef lt_eq_2 = LLVMBuildICmp(
    builder,
    LLVMIntSLE,
    x,
    two,
    ""
  );

  LLVMBuildCondBr(
    builder,
    lt_eq_2,
    ret,
    recur
  );

  // Create `return 1`
  LLVMPositionBuilderAtEnd(builder, ret);

  LLVMBuildRet(builder, one);

  // fib(x - 1)
  LLVMPositionBuilderAtEnd(builder, recur);

  LLVMValueRef x_min_1 = LLVMBuildSub(
    builder,
    x,
    one,
    ""
  );

  LLVMValueRef fib_x_min_1 = LLVMBuildCall2(
    builder,
    signature,
    fn,
    &x_min_1,
    1,
    ""
  );

  LLVMSetTailCall(fib_x_min_1, T);

  // fib(x - 2)
  LLVMValueRef x_min_2 = LLVMBuildSub(
    builder,
    x,
    two,
    ""
  );

  LLVMValueRef fib_x_min_2 = LLVMBuildCall2(
    builder,
    signature,
    fn,
    &x_min_2,
    1,
    ""
  );

  LLVMSetTailCall(fib_x_min_2, T);

  //
  LLVMValueRef sum = LLVMBuildAdd(
    builder,
    fib_x_min_1,
    fib_x_min_2,
    ""
  );

  LLVMBuildRet(
    builder,
    sum
  );

  // Cleanup
  LLVMDisposeBuilder(builder);

  return NULL;
}
