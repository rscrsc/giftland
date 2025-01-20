CFLAGS := -Wall -Wextra -g -std=c++20 -I/opt/VulkanSDK/1.4.304.0/macOS/include $(shell pkg-config --cflags sdl2)
LDFLAGS := -L/opt/VulkanSDK/1.4.304.0/macOS/lib -lvulkan $(shell pkg-config --libs --static sdl2)
SHADER_DIR := shader
SHADER_SRCS := $(wildcard $(SHADER_DIR)/*.glsl)

all: main shaders
	sh -c ". /opt/VulkanSDK/1.4.304.0/setup-env.sh 2>&1 1>/dev/null; \
	./main"

main: main.o Vulkan.o Sdl.o
	sh -c ". /opt/VulkanSDK/1.4.304.0/setup-env.sh 2>&1 1>/dev/null; \
	c++ $(CFLAGS) $(LDFLAGS) -o main main.o Vulkan.o Sdl.o"

main.o: main.cpp Vulkan.h Sdl.h Config.h utils.h
	sh -c ". /opt/VulkanSDK/1.4.304.0/setup-env.sh 2>&1 1>/dev/null; \
	c++ $(CFLAGS) -o main.o -c main.cpp"

Vulkan.o: Vulkan.cpp Vulkan.h Config.h utils.h
	sh -c ". /opt/VulkanSDK/1.4.304.0/setup-env.sh 2>&1 1>/dev/null; \
	c++ $(CFLAGS) -o Vulkan.o -c Vulkan.cpp"

Sdl.o: Sdl.cpp Sdl.h Vulkan.h Config.h utils.h
	sh -c ". /opt/VulkanSDK/1.4.304.0/setup-env.sh 2>&1 1>/dev/null; \
	c++ $(CFLAGS) -o Sdl.o -c Sdl.cpp"

shaders: $(SHADER_SRCS)
	./script/shaderc
