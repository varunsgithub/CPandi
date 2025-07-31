# Compiler and flags
CC = gcc
CFLAGS = -I.

# Source files and object files
DEPS = common.h debug.h chunk.h memory.h value.h vm.h compiler.h scanner.h
OBJ = main.o debug.o chunk.o memory.o value.o vm.o compiler.o scanner.o

# Default target
main: $(OBJ)
	$(CC) -o $@ $^

# Pattern rule for object files
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Clean target to remove binaries
destruct:
	rm -f *.o main