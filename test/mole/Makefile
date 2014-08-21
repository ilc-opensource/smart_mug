ROOT=../..
include $(ROOT)/common.mk

BIN_PATH=.
SRC_PATH=.
BUILD_PATH=build

## Edit #######################################
TARGET=$(BIN_PATH)/mole
SRCS=mole.cpp
###############################################

OBJS=$(addprefix $(BUILD_PATH)/, $(SRCS:.cpp=.o))

all: init $(TARGET) end

end:
	@echo "done"

init:
	@mkdir -p $(BUILD_PATH)

$(TARGET):$(OBJS) $(LIBMUG)
	$(CXX) $^ -o $@ $(LD_FLAGS)

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(C_FLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_PATH)
	rm -rf $(TARGET)

.PHONY: clean all




