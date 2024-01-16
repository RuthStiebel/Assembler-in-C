# Compiler settings
CC = gcc
CFLAGS = -g -ansi -Wall -pedantic

# Source files
SRCS =  directives.c generateOutput.c instructions.c labels.c main.c  parser.c preprocessor.c print.c 
OBJS = $(SRCS:.c=.o)
DEPS = directives.h generateOutput.h instructions.h labels.h parser.h preprocessor.h print.h utils.h

# Executable
TARGET = assembler

# Rule to compile .c files into .o files
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Default rule
all: $(TARGET)

# Rule to build the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
