MUG_ROOT=../..
INC_PATH=$(MUG_ROOT)/include
LIB_PATH=$(MUG_ROOT)/bin

BIN_PATH=bin
SRC_PATH=./
BUILD_PATH=build

## Edit #######################################
TARGET=$(BIN_PATH)/show
SRCS=show.c
###############################################

C_FLAGS=-O0 -g -I$(INC_PATH) -Iinclude
OBJS=$(addprefix $(BUILD_PATH)/, $(SRCS:.c=.o))

all: init $(TARGET) end

end:
	@echo "done"

init:
	@mkdir -p bin
	@mkdir -p build

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(LIB_PATH)/libmug.a

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) $(C_FLAGS) -c $< -o $@

clean:
	rm -rf build
	rm -rf bin

.PHONY: clean all




