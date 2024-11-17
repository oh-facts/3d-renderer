# valid args: clean, debug, release, clang, gcc 

cd "$(dirname "$0")"

for arg in "$@"; do declare $arg='1'; done

[ "$clean" == "1" ] && rm -f yk && rm -f *.o && rm -f libvma.a && rm -f *.spv && echo "cleaned" && exit 0
[ "$shader" == "1" ] && glslc hello_triangle.vert -o hello_triangle.vert.spv && glslc hello_triangle.frag -o hello_triangle.frag.spv && echo "[compiled shaders]"

debug_build="-g -O0"
release_build="-O2"
compiler="gcc"

build_type="$debug_build"
vulkan_sdk=""

if [[ "$(uname -s)" == "Darwin" ]]; then
  # default installation directory
  vulkan_sdk=`echo ${HOME}/VulkanSDK/*/macOS`
  if [ ! -e "$vulkan_sdk" ]; then
    if [ -n "$VULKAN_SDK" ]; then
      vulkan_sdk="$VULKAN_SDK"
    else  
      echo "vulkan sdk not found.\n    export VULKAN_SDK=path/to/VulkanSDK" && exit 1
    fi
  fi
fi

[ "$release" == "1" ] && build_type="$release_build"
[ "$clang" == "1" ] && compiler="clang"

echo "[$compiler]"
echo "[$build_type]"


[ "$ext" == "1" ] && g++ -std=c++17 $build_type -I. -I$vulkan_sdk/include/vma -c vma.cpp -o vma.o && echo "compiled vma"

$compiler -Wall -Wextra -Wno-unused-function -Wno-int-conversion -Wno-incompatible-pointer-types -Wno-sign-compare -Wno-unused-parameter -std=c99 -D_GNU_SOURCE $build_type -I. -I$vulkan_sdk/include/vma -c ./main.c -o main.o && echo "compiled yk"

g++ main.o vma.o -o yk -lm $(pkg-config --libs --static glfw3) && echo "linked yk and vma"
