CC = gcc
CFLAGS = -std=c99 \
         -D_POSIX_C_SOURCE=200809L \
         -D_XOPEN_SOURCE=700 \
         -Wall -Wextra -Werror \
         -Wno-unused-parameter \
         -fno-asm

# NEW: Add the include directory to the compiler flags
CFLAGS += -Iinclude

# NEW: Define source and object directories
SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include

# Find all .c files in the source directory
SRC = $(wildcard $(SRC_DIR)/*.c)
# Create a list of corresponding .o files in the object directory
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# List all header files
DEPS = $(wildcard $(INCLUDE_DIR)/*.h)

# The final executable name
TARGET = shell.out

# The default rule
all: $(TARGET)

# Rule to link the final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

# NEW: Rule to compile .c files from src/ into .o files in obj/
# This also ensures the object directory exists.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up compiled files
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean

