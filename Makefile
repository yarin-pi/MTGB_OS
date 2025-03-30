# Define the compiler, assembler, and linker
CC = i686-elf-gcc
LD = i686-elf-ld
NASM = nasm  # Use NASM as the assembler

# Compiler flags
CFLAGS = -ffreestanding -Wall -w -Wextra -I./Include -g -gdwarf-4 

# Linker flags
LDFLAGS_BIN = -T linker.ld --oformat=binary
LDFLAGS_ELF = -T linker2.ld

# List of source and assembly files
SRCS = $(wildcard *.c)
ASMS = switch_task.asm  # Add your NASM assembly file here

# Ensure kl.c is the first object file
OBJS = kl.o $(filter-out kl.o, $(SRCS:.c=.o)) $(ASMS:.asm=.o)

# Output file names
OUTPUT_BIN = kernel.bin
OUTPUT_ELF = kernel.elf

# Default target: build both binary and ELF
all: $(OUTPUT_BIN) $(OUTPUT_ELF)

# Rule to build the binary file
$(OUTPUT_BIN): $(OBJS)
	$(LD) $(LDFLAGS_BIN) -o $@ $(OBJS)

# Rule to build the ELF file with debug symbols
$(OUTPUT_ELF): $(OBJS)
	$(LD) $(LDFLAGS_ELF) -o $@ $(OBJS)

# Pattern rule for compiling .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for assembling .asm files to .o files using NASM
%.o: %.asm
	$(NASM) -f elf -o $@ $<  

# Clean up generated files
clean:
	rm -f $(OBJS) $(OUTPUT_BIN) $(OUTPUT_ELF)

# Rebuild everything
rebuild: clean all
