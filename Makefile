# Define the compiler and linker
CC = i686-elf-gcc
LD = i686-elf-ld

# Compiler flags
CFLAGS = -ffreestanding -Wall -w -Wextra -I./Include

# Linker flags
LDFLAGS = -T linker.ld --oformat=binary

# List of object files
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

# Output binary name
OUTPUT = kernel.bin

# Default target
all: $(OUTPUT)

# Rule to build the kernel binary
$(OUTPUT): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# Pattern rule for compiling .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJS) $(OUTPUT)

# Rebuild everything
rebuild: clean all
