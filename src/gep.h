#include <llvm-c/Core.h>

typedef struct {
  int f1;
  int f2;
} Munger;

LLVMValueRef create_get_snd_int_fn (
  LLVMContextRef ctx,
  LLVMModuleRef mod,
  const char* name,
  int num_bits
);

LLVMValueRef create_munge_fn (
  LLVMContextRef ctx,
  LLVMModuleRef mod,
  const char* name,
  int num_bits
);
