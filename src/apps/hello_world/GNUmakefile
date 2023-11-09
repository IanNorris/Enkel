TOOLS := ../../../tools

# Define compiler and flags
CXX := g++
CXXFLAGS := -g3 -O0 -D_DEBUG -Wall -Wextra -pedantic -std=c++17 -static -m64
LDFLAGS := -nostdlib

NASMFLAGS := -Wall -f elf64

# Name of the executable
TARGET := hello_world.a

# Source and object files
SRC := hello_world.cpp
override NASMFILES := $(shell find -L . -type f -name '*.asm')
OBJ := $(SRC:.cpp=.o) $(NASMFILES:.asm=.asm.o)

# Default target
all: $(TARGET)

# Link the target with object files
$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^
	python3 $(TOOLS)/bin2c.py $@ $(basename $@).bin

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.asm.o: %.asm
	nasm $(NASMFLAGS) -l $(basename $<).lst $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJ) $(TARGET)

# Non-file targets
.PHONY: all clean
