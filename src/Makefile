LLVM     = ../deps/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04/
LLVM_BIN = $(LLVM)bin/
LLVM_INC = $(LLVM)include/llvm-c/

CC      = $(LLVM_BIN)clang
CFLAGS  = -g -I$(LLVM_INC) `$(LLVM_BIN)llvm-config --cflags`
LD      = $(LLVM_BIN)clang++
LDFLAGS = *.o `$(LLVM_BIN)llvm-config --cxxflags --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter --system-libs`

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
BC  = $(SRC:.c=.bc)
LL  = $(SRC:.c=.ll)

main: $(OBJ)
	$(LD) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.ll: %.bc
	llvm-dis $<

.PHONY: clean
clean:
	-rm -f main $(OBJ) $(BC) $(LL)
