// LLVM IR for c-like language w/ fn:
//
//  int sum (int x, int y)
//  {
//    return x + y;
//  }

#include "sum.h"
#include "util.h"

LLVMValueRef create_int_sum_fn (
  LLVMContextRef ctx,
  LLVMModuleRef mod,
  const char* name,
  unsigned num_bits
)
{
  // Types
  LLVMTypeRef int_type = LLVMIntTypeInContext(ctx, num_bits);

  // New fn: sum (Int, Int) Int
  unsigned num_params       = 2;
  LLVMTypeRef param_types[] = { int_type, int_type };
  LLVMTypeRef return_type   = int_type;
  LLVMTypeRef signature     = LLVMFunctionType(return_type, param_types, num_params, F);
  LLVMValueRef fn           = LLVMAddFunction(mod, name, signature);

  // Get args
  LLVMValueRef x = LLVMGetParam(fn, 0);
  LLVMValueRef y = LLVMGetParam(fn, 1);

  // Basic block
  // - 1 entry, 1 exit
  // - only way to exec is series of statements in order
  // - no if/else, loops, or jumps of any kind
  // - core to modeling control flow and later optimizations
  // - 
  LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(ctx, fn, "entry");

  // Position builder to start where we left off in 'entry' block
  LLVMBuilderRef builder = LLVMCreateBuilderInContext(ctx);
  LLVMPositionBuilderAtEnd(builder, entry);

  // Get values and apply to 'tmp'
  LLVMValueRef tmp = LLVMBuildAdd(builder, x, y, "tmp");
  LLVMBuildRet(builder, tmp); // Generate return statement

  // Cleanup
  LLVMDisposeBuilder(builder);

  return fn;
}
