// LLVM IR for c-like language w/ fn:
//
// int sum (int a, int b)
// {
//   return a + b;
// }

#include "sum.h"
#include "util.h"

LLVMValueRef create_int_sum_fn (
  LLVMContextRef ctx,
  LLVMModuleRef mod,
  const char* name,
  unsigned num_bits
)
{
  // New fn: sum (Int32, Int32) Int32
  LLVMTypeRef param_types[] = { LLVMIntTypeInContext(ctx, num_bits), LLVMIntTypeInContext(ctx, num_bits) }; // params
  LLVMTypeRef return_type   = LLVMIntTypeInContext(ctx, num_bits);                                          // return type
  LLVMTypeRef signature     = LLVMFunctionType(return_type, param_types, 2, F);                             // Declared function
  LLVMValueRef fn           = LLVMAddFunction(mod, name, signature);                                        // Adds fn to module (concrete/memory)

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
  LLVMValueRef tmp = LLVMBuildAdd(builder, LLVMGetParam(fn, 0), LLVMGetParam(fn, 1), "tmp");
  LLVMBuildRet(builder, tmp); // Generate return statement

  // Cleanup
  LLVMDisposeBuilder(builder);

  return fn;
}
