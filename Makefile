# Compiler
# CC = $(shell which gcc-14 || echo gcc) # Prefer gcc-14 if available
# Compilers
CC =gcc-openmp -fopenmp 
CXX = g++-openmp -fopenmp 
CFLAGS = -g3 -Wall -lstdc++ -Wextra
CXXFLAGS = $(CFLAGS) -fpermissive  # C++ standard can be adjusted as needed

# Directories
SRC_DIR = src
OBJ_DIR = obj
TARGET_DIR = targets
BOX2D = ~/repo_packages/box2d
INCLUDES = -I $(BOX2D)/include -I includes 
LIBS = -L $(BOX2D)/build/src -lbox2d -lraylib -lm -pthread -ldl -lrt -lX11

# Source and object files
C_SRCS = $(wildcard $(SRC_DIR)/*.c)
CPP_SRCS = $(wildcard $(SRC_DIR)/*.cpp)
C_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_SRCS))
CPP_OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CPP_SRCS))
OBJS = $(C_OBJS) $(CPP_OBJS)

# Target executables might be in C or C++, adjust suffix matching accordingly
C_TARGETS = $(patsubst $(TARGET_DIR)/%.c, %.out, $(wildcard $(TARGET_DIR)/*.c))
CPP_TARGETS = $(patsubst $(TARGET_DIR)/%.cpp, %.out, $(wildcard $(TARGET_DIR)/*.cpp))
TARGETS = $(C_TARGETS) $(CPP_TARGETS)

# Default target
all: $(TARGETS)

# Generate an object file for each C target instead of an executable
C_TARGET_OBJS = $(patsubst $(TARGET_DIR)/%.c, $(OBJ_DIR)/%.target.o, $(wildcard $(TARGET_DIR)/*.c))

# Link each C target executable
$(C_TARGETS): %.out: $(OBJS) $(OBJ_DIR)/%.target.o
	$(CXX) $(CXXFLAGS) $(OBJS) $(OBJ_DIR)/$*.target.o -o $@ $(INCLUDES) $(LIBS)

# Compile each C target source file into a separate object file (containing main)
$(C_TARGET_OBJS): $(OBJ_DIR)/%.target.o: $(TARGET_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)


# # Link each C++ target executable
$(CPP_TARGETS): %.out: $(OBJS) $(TARGET_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(OBJS) $(TARGET_DIR)/$*.cpp -o $@ $(INCLUDES) $(LIBS)

# Compile each C source file to an object file
$(C_OBJS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)

# Compile each C++ source file to an object file
$(CPP_OBJS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDES)

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up generated files
clean:
	rm -f *.out
	rm -rf $(OBJ_DIR)

# Phony targets
.PHONY: all clean
