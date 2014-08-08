MUG_ROOT=../..
INC_PATH=$(MUG_ROOT)/include
LIB_PATH=$(MUG_ROOT)/bin

BIN_PATH=bin
SRC_PATH=./
BUILD_PATH=build

## Edit #######################################
TARGET=$(BIN_PATH)/touch
SRCS=touch.cpp
###############################################

C_FLAGS=-O0 -g -I$(INC_PATH) -Iinclude
LD_FLAGS=-L../../lib/libuv/out/Debug -L../../lib/libuv/out/Release -luv -lpthread -lrt
OBJS=$(addprefix $(BUILD_PATH)/, $(SRCS:.cpp=.o))

all: init $(TARGET) end

end:
	@echo "done"

init:
	@mkdir -p bin
	@mkdir -p build

$(TARGET):$(OBJS)
	$(CXX) $^ -o $@ $(LIB_PATH)/libmug.a $(LD_FLAGS)

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(C_FLAGS) -c $< -o $@

clean:
	rm -rf build
	rm -rf bin

.PHONY: clean all




