OUT := cel
LIB_FLAGS :=-lm

CC := gcc
PP_FLAGS := 
CC_FLAGS := -O3 -Wall -MMD -MP
SRC_DIR := src
INC_DIR := include
OBJ_DIR := build

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC_FILES := $(call rwildcard,$(SRC_DIR)/,*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
INC_FLAGS := $(addprefix -I,$(INC_DIR))

all: $(OUT)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OUT): $(OBJ_DIR) $(OBJ_FILES)
	$(CC) -o $(OUT) $(LIB_FLAGS) $(OBJ_FILES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(INC_FLAGS) $(PP_FLAGS) $(CC_FLAGS) -c "$<" -o "$@"

-include $(OBJ_FILES:.o=.d)

test: $(OUT)
	./$(OUT)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(OUT)

.PHONY:
	all clean test
