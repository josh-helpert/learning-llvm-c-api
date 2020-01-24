#include <llvm-c/Core.h>

LLVMValueRef create_fib_fn (
  LLVMContextRef ctx,
  LLVMModuleRef mod,
  const char* name,
  unsigned num_bits
);
