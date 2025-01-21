CFLAGS := -Wall -Wextra -g -O3 -std=c++20 -I/opt/VulkanSDK/1.4.304.0/macOS/include $(shell pkg-config --cflags sdl2)
LDFLAGS := -L/opt/VulkanSDK/1.4.304.0/macOS/lib -lvulkan $(shell pkg-config --libs --static sdl2)
SHADER_DIR := shader
SHADER_SRCS := $(wildcard $(SHADER_DIR)/*.glsl)
BUILD_DIR := build
SRC_DIR := src
H_SRCS := $(wildcard $(SRC_DIR)/*.h)
obj-y := main.o Sdl.o Vulkan.o
OBJS := $(addprefix $(BUILD_DIR)/, $(obj-y))

all: build/main shaders
	sh -c ". /opt/VulkanSDK/1.4.304.0/setup-env.sh 2>&1 1>/dev/null; \
	build/main"

build/main: $(OBJS)
	sh -c ". /opt/VulkanSDK/1.4.304.0/setup-env.sh 2>&1 1>/dev/null; \
	c++ $(CFLAGS) $(LDFLAGS) -o $@ $^"

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp $(H_SRCS)
	sh -c ". /opt/VulkanSDK/1.4.304.0/setup-env.sh 2>&1 1>/dev/null; \
	c++ $(CFLAGS) -o $@ -c $(SRC_DIR)/main.cpp"

$(BUILD_DIR)/Vulkan.o: $(SRC_DIR)/Vulkan.cpp $(H_SRCS)
	sh -c ". /opt/VulkanSDK/1.4.304.0/setup-env.sh 2>&1 1>/dev/null; \
	c++ $(CFLAGS) -o $@ -c $(SRC_DIR)/Vulkan.cpp"

$(BUILD_DIR)/Sdl.o: $(SRC_DIR)/Sdl.cpp $(H_SRCS)
	sh -c ". /opt/VulkanSDK/1.4.304.0/setup-env.sh 2>&1 1>/dev/null; \
	c++ $(CFLAGS) -o $@ -c $(SRC_DIR)/Sdl.cpp"

shaders:
	./script/shaderc
