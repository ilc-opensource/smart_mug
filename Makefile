ROOT=.
include common.mk
IOHUB_ROOT= $(ROOT)/lib/edison

C_FLAGS+=-I$(IOHUB_ROOT)/lib/include
LIB_PATH=$(IOHUB_ROOT)/lib/bin

BIN_PATH=bin
SRC_PATH=src
BUILD_PATH=build
TARGET=$(BIN_PATH)/libmug.a

SRCS=disp.cpp image.cpp mug.cpp motion.cpp touch.cpp adc.cpp res_manager.cpp io.cpp utf8.cpp cJSON.cpp config.cpp

OBJS=$(addprefix $(BUILD_PATH)/, $(SRCS:.cpp=.o))

all: init $(TARGET) end

end:
	@echo "done"

init:
	@mkdir -p bin
	@mkdir -p build

$(TARGET):$(OBJS)
	cp $(LIB_PATH)/libiohub-client.a $(TARGET)
	$(AR) -rc $@ $^

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(C_FLAGS) -c $< -o $@

test:
	make -C test

clean: clean_test
	rm -rf build
	rm -rf bin
	make clean -C test

clean_test:
	make clean -C test

.PHONY: clean all test




