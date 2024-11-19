# valid args: clean, debug, release, clang, gcc, ext, shader

cd "$(dirname "$0")"

for arg in "$@"; do declare $arg='1'; done

[ "$clean" == "1" ] && rm -f yk && rm -f *.o && rm -f libvma.a && rm -f *.spv && echo "cleaned" && exit 0
[ "$shader" == "1" ] && glslc hello_triangle.vert -o hello_triangle.vert.spv && glslc hello_triangle.frag -o hello_triangle.frag.spv && echo "compiled shaders"

debug_build="-g -O0"
release_build="-O2"
c_compiler="clang"
cpp_compiler="clang++"

build_type="$debug_build"

[ "$release" == "1" ] && build_type="$release_build"
[ "$gcc" == "1" ] && c_compiler="gcc" && cpp_compiler="g++"

echo "[$c_compiler]"
echo "[$cpp_compiler]"
echo "[$build_type]"

[ "$ext" == "1" ] && $cpp_compiler -std=c++17 $build_type -I. -c vma.cpp -o vma.o && echo "compiled vma"

$c_compiler -Wall -Wextra -Wno-unused-function -Wno-int-conversion -Wno-incompatible-pointer-types -Wno-sign-compare -Wno-unused-parameter -std=c99 -D_GNU_SOURCE $build_type -I. -c ./main.c -o main.o && echo "compiled yk"

$cpp_compiler main.o vma.o -o yk -lm -lglfw && echo "linked yk and vma"