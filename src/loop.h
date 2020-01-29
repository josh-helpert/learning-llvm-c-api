#include <llvm-c/Core.h>

void print_arr (
  const char *name, 
  double* ptr, 
  size_t elements
);

LLVMValueRef create_loop_fn (
  LLVMContextRef ctx,
  LLVMModuleRef mod,
  const char* name
);
