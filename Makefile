CC := gcc
CFLAGS := -Wall -Werror -Wextra -Iinclude -g 
SRC = $(shell find src -name "*.c")
OBJ = $(SRC:src/%.c=build/%.o)
TARGET = build/my_program

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf build