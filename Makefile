# Compiler
CC = $(shell which gcc-14 || echo gcc) # Prefer gcc-14 if available
CFLAGS = -g2 -Wall -Wextra # -fno-strict-aliasing

# Directories
SRC_DIR = src
OBJ_DIR = obj
TARGET_DIR = targets
BOX2D = ~/repo_packages/box2d
INCLUDES = -I $(BOX2D)/include -I includes
LIBS = -L $(BOX2D)/build/src -lbox2d -lraylib -lm -pthread -ldl -lrt -lX11

# Source and object files
# SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(SRC_DIR)/*.c))

# Target executable in the root directory with .out postfix
TARGETS = $(patsubst $(TARGET_DIR)/%.c, %.out, $(wildcard $(TARGET_DIR)/*.c))

# Default target
all: $(TARGETS)

# Link each target executable
%.out: $(OBJS) $(TARGET_DIR)/%.c
	$(CC) $(CFLAGS) $(OBJS) $(TARGET_DIR)/$*.c -o $@ $(INCLUDES) $(LIBS)

# Compile each source file to an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up generated files
clean:
	rm -f *.out
	rm -rf $(OBJ_DIR)

# Phony targets
.PHONY: all clean
