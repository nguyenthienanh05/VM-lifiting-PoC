CXX := clang++
LLVM_CONFIG ?= /opt/homebrew/opt/llvm/bin/llvm-config

TARGET := disassembler
BUILD_DIR := build

SRCS := main.cpp src/disassembler.cpp src/lifter.cpp
OBJS := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

LLVM_CXXFLAGS := $(filter-out -fno-exceptions,$(shell $(LLVM_CONFIG) --cxxflags))
LLVM_LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags --system-libs --libs core)

CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wno-unused-parameter -g $(LLVM_CXXFLAGS)
LDFLAGS := $(LLVM_LDFLAGS)

.PHONY: all clean run check-llvm

all: check-llvm $(TARGET)

check-llvm:
	@test -x "$(LLVM_CONFIG)" || \
		(echo "llvm-config not found at $(LLVM_CONFIG). Set LLVM_CONFIG=/path/to/llvm-config"; exit 1)

$(TARGET): $(OBJS)
	$(CXX) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	./$(TARGET) command

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)
