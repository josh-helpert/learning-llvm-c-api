#include <llvm-c/Core.h>

LLVMValueRef create_int_sum_fn (
  LLVMContextRef ctx,
  LLVMModuleRef mod,
  const char* name,
  unsigned num_bits
);
