#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "sum.h"
#include "fib.h"

typedef struct {
  LLVMContextRef ctx;
  LLVMModuleRef mod;
  const char* name;
} LLVMCtx;

static LLVMValueRef add_fn_signature (
  const char* name, 
  LLVMModuleRef mod, 
  LLVMTypeRef params[], 
  LLVMTypeRef ret, 
  int num_params, 
  int is_variadic
)
{
  LLVMTypeRef signature = LLVMFunctionType(ret, params, num_params, is_variadic); // Declared function
  LLVMValueRef fn       = LLVMAddFunction(mod, name, signature);                   // Adds fn to module (concrete/memory)
  return fn;
}

static int init_env ()
{
  // Initialize
  LLVMLinkInMCJIT();
  LLVMLinkInInterpreter();
  LLVMInitializeNativeTarget();
  LLVMInitializeNativeAsmPrinter();

  // Get triple
  char* triple = LLVMGetDefaultTargetTriple();
  LLVMTargetRef target_ref;
  char* err;

  if (LLVMGetTargetFromTriple(triple, &target_ref, &err))
  {
    printf("Error: %s\n", err);
    return 1;
  }

  // 
  LLVMTargetMachineRef tm_ref = LLVMCreateTargetMachine(
    target_ref,              // 
    triple,                  // 
    "",                      // const char* cpu
    "",                      // const char* features
    LLVMCodeGenLevelDefault, // level
    LLVMRelocStatic,         // reloc
    LLVMCodeModelJITDefault  // code model
  );
  LLVMDisposeMessage(triple);

  return 0;
}

int main (int argc, char const* argv[])
{
  // Initialize
  int ret = init_env();
  if (ret) return ret;

  //--- Build LLVM IR

  // New ctx
  LLVMContextRef ctx = LLVMContextCreate();

  // New module
  //LLVMModuleRef mod = LLVMModuleCreateWithName("my_module"); // Implicitly global ctx
  LLVMModuleRef mod = LLVMModuleCreateWithNameInContext("my_module", ctx);

  // Add functions
  LLVMValueRef sum_fn = create_int_sum_fn(ctx, mod, "sum", 32);
  LLVMValueRef fib_fn = create_fib_fn(ctx, mod, "fib", 32);

  //--- Analysis and execution

  // Verify the module
  char* err = NULL;

  LLVMVerifyModule(mod, LLVMAbortProcessAction, &err);
  LLVMDisposeMessage(err);

  // Build executor
  err                           = NULL;
  LLVMExecutionEngineRef engine = NULL;

  if (LLVMCreateExecutionEngineForModule(&engine, mod, &err) != 0)
  {
    fprintf(stderr, "Failed to create execution engine\n");
    abort();
  }

  if (err)
  {
    fprintf(stderr, "Error: %s\n", err);
    LLVMDisposeMessage(err);
    exit(EXIT_FAILURE);
  }

  // Parse args
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s Int32 x Int32 y\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  long long x = strtoll(argv[1], NULL, 10);
  long long y = strtoll(argv[2], NULL, 10);

  // Get functions
  int (*sum)(int, int) = (int (*)(int, int))LLVMGetFunctionAddress(engine, "sum");
  printf("result: %d\n", sum(x, y));

  int (*fib)(int) = (int (*)(int))LLVMGetFunctionAddress(engine, "fib");
  printf("result: %d\n", fib(x));

  // Test
  printf("--- testing sum fn ---\n");
  printf("\tsum 0 0: %d\n", sum(0, 0));
  printf("\tsum 0 1: %d\n", sum(0, 1));
  printf("\tsum 1 0: %d\n", sum(1, 0));
  printf("\tsum 1 1: %d\n", sum(1, 1));
  printf("----------------------\n");

  printf("--- testing fib fn ---\n");
  printf("\tfib 0:   %d\n", fib(0));
  printf("\tfib 1:   %d\n", fib(1));
  printf("\tfib 10:  %d\n", fib(10));
  printf("\tfib 100: %d\n", fib(100));
  printf("----------------------\n");

  // Write bitcode
  if (LLVMWriteBitcodeToFile(mod, "sum.bc") != 0)
  {
    fprintf(stderr, "Failed to write 'sum' bitcode to file, skipping...\n");
  }

  if (LLVMWriteBitcodeToFile(mod, "fib.bc") != 0)
  {
    fprintf(stderr, "Failed to write 'fib' bitcode to file, skipping...\n");
  }

  // Dump module
  fprintf(stderr, "\n--- Module ---\n");

  LLVMDumpModule(mod);

  fprintf(stderr, "--------------\n");

  // Cleanup
  LLVMDisposeModule(mod);
  LLVMContextDispose(ctx);
}
