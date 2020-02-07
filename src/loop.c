// LLVM equivalent of:
//
//  void loop (double *result, double *x, double *y, size_t length)
//  {
//    for (int32_t i = 0; i < length; i++)
//    {
//      result[i] = x[i] * y[i];
//    }
//  }

// Adapted from: https://gist.github.com/Mytherin/ccd6dd333258bdf409f9525a7a35be36
// - Use updated LLVM API methods
// - Added more comments
// - Load arguments once
// - 

#include "loop.h"
#include "util.h"

void print_arr (
  const char *name, 
  double* ptr, 
  size_t elements
)
{
  printf("%s: [", name);

  for (size_t i = 0; i < elements; i++)
  {
    printf("%lf", ptr[i]);

    if (i != elements - 1)
    {
      printf(", ");
    }
  }

  printf("]\n");
}

LLVMValueRef create_loop_fn (
  LLVMContextRef ctx,
  LLVMModuleRef mod,
  const char* name
)
{
  // Types
  LLVMTypeRef dbl_type     = LLVMDoubleTypeInContext(ctx);
  LLVMTypeRef dbl_ptr_type = LLVMPointerType(dbl_type, 0 /* AddressSpace */);
  LLVMTypeRef int64_type   = LLVMInt64TypeInContext(ctx);

  // Function
  unsigned num_params       = 4;
  LLVMTypeRef param_types[] = { dbl_ptr_type, dbl_ptr_type, dbl_ptr_type, int64_type };
  LLVMTypeRef return_type   = LLVMVoidTypeInContext(ctx);
  LLVMTypeRef signature     = LLVMFunctionType(return_type, param_types, num_params, F);
  LLVMValueRef fn           = LLVMAddFunction(mod, name, signature);

  // Consts
  LLVMValueRef zero = LLVMConstInt(int64_type, 0, T /* sign extended */);
  LLVMValueRef one  = LLVMConstInt(int64_type, 1, T /* sign extended */);

  // Params
  LLVMValueRef arg_result = LLVMGetParam(fn, 0);
  LLVMValueRef arg_ptr_x  = LLVMGetParam(fn, 1);
  LLVMValueRef arg_ptr_y  = LLVMGetParam(fn, 2);
  LLVMValueRef arg_len    = LLVMGetParam(fn, 3);

  // Create blocks
  LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(ctx, fn, "entry");
  LLVMBasicBlockRef cond  = LLVMAppendBasicBlockInContext(ctx, fn, "cond");
  LLVMBasicBlockRef body  = LLVMAppendBasicBlockInContext(ctx, fn, "body");
  LLVMBasicBlockRef inc   = LLVMAppendBasicBlockInContext(ctx, fn, "inc");
  LLVMBasicBlockRef end   = LLVMAppendBasicBlockInContext(ctx, fn, "end");

  // Create and position builder
  LLVMBuilderRef builder = LLVMCreateBuilderInContext(ctx);

  // Entry
  //   represents: i = 0
  LLVMValueRef i_addr = NULL;

  {
    LLVMPositionBuilderAtEnd(builder, entry);

    // Allocate (on stack) i64 i
    i_addr = LLVMBuildAlloca(builder, int64_type, "i");;

    // Store 0 in i
    LLVMBuildStore(builder, zero, i_addr);

    // Branch to condition
    LLVMBuildBr(builder, cond);
  }
  
  // Condition
  //   represents: i < length
  {
    LLVMPositionBuilderAtEnd(builder, cond);

    // Load i
    LLVMValueRef i = LLVMBuildLoad2(builder, int64_type, i_addr, "");

    // Build condition (i < length)
    LLVMValueRef cond = LLVMBuildICmp(
      builder,
      LLVMIntSLE,
      i,
      arg_len,
      ""
    );

    // Branch to T:body or F:end
    LLVMBuildCondBr(
      builder,
      cond,
      body,
      end
    );
  }

  // Body
  //   represents: result[i] = x[i] * y[i];
  {
    LLVMPositionBuilderAtEnd(builder, body);

    // Load i
    LLVMValueRef i        = LLVMBuildLoad2(builder, int64_type, i_addr, "");
    LLVMValueRef* indexes = &i;
    int num_indexes       = 1;

    // Get x[i]
    LLVMValueRef x_addr = LLVMBuildGEP2(builder, dbl_type, arg_ptr_x, indexes, num_indexes, "");
    LLVMValueRef x_i    = LLVMBuildLoad2(builder, dbl_type, x_addr, "");

    // Get y[i]
    LLVMValueRef y_addr = LLVMBuildGEP2(builder, dbl_type, arg_ptr_y, indexes, num_indexes, "");
    LLVMValueRef y_i    = LLVMBuildLoad2(builder, dbl_type, y_addr, "");

    // Multiply
    LLVMValueRef x_mul_y = LLVMBuildFMul(builder, x_i, y_i, "");

    // Get result[i] address
    LLVMValueRef result_addr = LLVMBuildGEP2(builder, dbl_type, arg_result, indexes, num_indexes, "");

    // Store in result[i]
    LLVMBuildStore(builder, x_mul_y, result_addr);

    // Branch
    LLVMBuildBr(builder, inc);
  }

  // Increment
  //   represents: i++
  LLVMPositionBuilderAtEnd(builder, inc);

  {
    // Increment i
    LLVMValueRef i       = LLVMBuildLoad2(builder, int64_type, i_addr, "");
    LLVMValueRef i_add_1 = LLVMBuildAdd(builder, i, one, "");

    LLVMBuildStore(builder, i_add_1, i_addr);

    // Branch
    LLVMBuildBr(builder, cond);
  }

  // End
  LLVMPositionBuilderAtEnd(builder, end);

  {
    LLVMBuildRetVoid(builder);
  }

  return fn;
}
