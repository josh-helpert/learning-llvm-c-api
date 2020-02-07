// LLVM for c-like language w/ fn:
//
//  int fib (int x)
//  {
//    if (x <= 2) return 1;
//    return fib(x - 1) + fib(x - 2);
//  }

#include "fib.h"
#include "util.h"

LLVMValueRef create_fib_fn (
  LLVMContextRef ctx,
  LLVMModuleRef mod,
  const char* name,
  unsigned num_bits
)
{
  // Types
  LLVMTypeRef int_type = LLVMIntTypeInContext(ctx, num_bits);

  // Build fn
  unsigned num_params       = 1;
  LLVMTypeRef param_types[] = { int_type };
  LLVMTypeRef return_type   = int_type;
  LLVMTypeRef signature     = LLVMFunctionType(return_type, param_types, num_params, F);
  LLVMValueRef fn           = LLVMAddFunction(mod, name, signature);

  LLVMSetLinkage(fn, LLVMExternalLinkage);

  //
  LLVMValueRef one = LLVMConstInt(int_type, 1, F);
  LLVMValueRef two = LLVMConstInt(int_type, 2, F);
  
  LLVMValueRef x = LLVMGetParam(fn, 0);
  //x->set_name    = "x";

  // Create blocks
  LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(ctx, fn, "entry");
  LLVMBasicBlockRef base  = LLVMAppendBasicBlockInContext(ctx, fn, "base");
  LLVMBasicBlockRef recur = LLVMAppendBasicBlockInContext(ctx, fn, "recur");

  // Create and position builder
  LLVMBuilderRef builder = LLVMCreateBuilderInContext(ctx);
  LLVMPositionBuilderAtEnd(builder, entry);

  // Create `if (x <= 2) goto base`
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
    base,
    recur
  );

  // Create `return 1`
  LLVMPositionBuilderAtEnd(builder, base);

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
