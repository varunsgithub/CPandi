# Compiler and flags
CC = gcc
CFLAGS = -I.

# Source files and object files
DEPS = common.h debug.h chunk.h memory.h value.h
OBJ = main.o debug.o chunk.o memory.o value.o

# Default target
main: $(OBJ)
	$(CC) -o $@ $^

# Pattern rule for object files
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Clean target to remove binaries
clean:
	rm -f *.o main