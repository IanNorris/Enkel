TOOLS := ../../../tools

# Define compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -pedantic -std=c++17 -static
LDFLAGS := -nostdlib

# Name of the executable
TARGET := hello_world.a

# Source and object files
SRC := hello_world.cpp
OBJ := $(SRC:.cpp=.o)

# Default target
all: $(TARGET)

# Link the target with object files
$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^
	python3 $(TOOLS)/bin2c.py $@ $(basename $@).bin

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJ) $(TARGET)

# Non-file targets
.PHONY: all clean
