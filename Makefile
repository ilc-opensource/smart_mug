ROOT=.
include common.mk
IOHUB_ROOT= $(ROOT)/lib/edison

C_FLAGS+=-I$(IOHUB_ROOT)/lib/include
LIB_PATH=$(IOHUB_ROOT)/lib/bin

BIN_PATH=bin
SRC_PATH=src
BUILD_PATH=build
TARGET=$(BIN_PATH)/libmug.a

NODE_TARGET=$(BIN_PATH)/libmug_node.a

SRCS=disp.cpp image.cpp mug.cpp motion.cpp touch.cpp adc.cpp res_manager.cpp io.cpp utf8.cpp cJSON.cpp config.cpp bme280.cpp bme280_support.cpp vaselib.cpp

OBJS=$(addprefix $(BUILD_PATH)/, $(SRCS:.cpp=.o))
NODE_OBJS= $(addprefix $(BUILD_PATH)/, $(SRCS:.cpp=_node.o))

all: init $(TARGET) $(NODE_TARGET) end

end:
	@echo "done"

init:
	@mkdir -p bin
	@mkdir -p build

$(TARGET):$(OBJS)
	cp $(LIB_PATH)/libiohub-client.a $@
	$(AR) -rc $@ $^

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(C_FLAGS) -c $< -o $@


$(NODE_TARGET): $(NODE_OBJS)
	cp $(LIB_PATH)/libiohub-client.a $@
	$(AR) -rc $@ $^

$(BUILD_PATH)/%_node.o: $(SRC_PATH)/%.cpp
	$(CXX) $(NODE_C_FLAGS) -c $< -o $@

node: init $(NODE_TARGET)

native: init $(TARGET)

test:
	make -C test

clean: clean_test
	rm -rf build
	rm -rf bin
	make clean -C test

clean_test:
	make clean -C test

.PHONY: clean all test




