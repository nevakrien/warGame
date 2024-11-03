# Compiler
# CC = $(shell which gcc-14 || echo gcc) # Prefer gcc-14 if available
CC =gcc
CFLAGS = -g2 -Wall -Wextra -g2 -fsanitize=undefined -fsanitize=address -fno-strict-aliasing

# Target executable
TARGET = game.out

# Directories
SRC_DIR = src
OBJ_DIR = obj
BOX2D = ~/repo_packages/box2d
INCLUDES = -I $(BOX2D)/include -I includes
LIBS = -L $(BOX2D)/build/src -lbox2d -lraylib -lm -pthread -ldl -lrt -lX11

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default target
all: $(TARGET)

# Link target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(INCLUDES) $(LIBS)

# Compile each source file to an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up generated files
clean:
	rm -f $(TARGET)
	rm -rf $(OBJ_DIR)

# Phony targets
.PHONY: all clean
