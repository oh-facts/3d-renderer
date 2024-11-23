# valid commands: <default>, all, clean
# valid args:
#   build_type=<debug|release>
#   compiler=<clang|gcc> 
#   compiler_cpp=<clang++|g++>
#   ext=1
#   shader=1

debug_build := -g -O0
release_build := -O2
compiler ?= clang
compile_cpp ?= clang++

build_type ?= debug

ext ?=
shader ?=
build_flags =

ifeq ($(VULKAN_SDK),)
$(error Vulkan path not set. Export: export VULKAN_SDK=path/to/VulkanSDK/*/macOS)
endif # VULKAN_SDK

vulkan_include := $(VULKAN_SDK)/include/vma
flags = $(strip -Wall -Wextra -Wno-unused-function -Wno-int-conversion \
	-Wno-incompatible-pointer-types -Wno-sign-compare -Wno-unused-parameter \
	-std=c99 -D_GNU_SOURCE -I$(vulkan_include))

ifeq ($(build_type),debug)
	build_flags = $(debug_build)
endif # debug
ifeq ($(build_type),release)
	build_flags = $(release_build)
endif # release
ifeq ($(compiler),clang)
	compiler = clang
	compiler_cpp = clang++
endif # clang
ifeq ($(compiler),gcc)
	compiler = gcc
	compiler_cpp = g++
endif # gcc

$(info [$(compiler)])
$(info [$(build_type)])

.PHONY: all clean
all:

ifeq ($(shader),1)
	$(foreach file,$(wildcard gpu/*.vert gpu/*.frag),$(shell glslc $(file) -o $(basename $(file)).spv))
	@echo "compiled shaders"
endif # shader
ifeq ($(ext),1)
	$(compiler_cpp) -std=c++17 $(build_flags) -I$(vulkan_include) -I. -c src/vma_impl.cpp -o vma.o
	@echo "compiled vma"
endif # ext
	$(compiler) $(flags) $(build_flags) -I. -c src/main.c -o main.o
	@echo "compiled yk"
	$(compiler_cpp) -I$(vulkan_include) main.o vma.o -o yk -Wl,-rpath,/usr/local/lib -lm -lglfw3 -framework Cocoa -framework IOKit -framework CoreFoundation -framework QuartzCore -lvulkan
	@echo "linked yk and vma"
clean:
	rm -f yk libvma.a *.o *.spv
	@echo "cleaned"
