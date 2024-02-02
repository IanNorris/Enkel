# Define compiler and flags
CXX := g++
CXXFLAGS := -g3 -O0 -D_DEBUG -Wall -Wextra -pedantic -std=c++17 -m64 -static-libgcc -static-libstdc++

CURRENT_DIR := $(shell basename $(CURDIR))

# Name of the executable
TARGET := $(CURRENT_DIR).a

# Source and object files
SRC := $(shell find -L . -type f -name '*.cpp')
OBJ := $(SRC:.cpp=.o)

# Default target
all: $(TARGET)

# Compile source files into target
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) -static -o $@ $^

# Clean up build artifacts
clean:
	rm -f $(OBJ) $(TARGET)

# Non-file targets
.PHONY: all clean
