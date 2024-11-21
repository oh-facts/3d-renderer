# valid args: clean, debug, release, clang, gcc, ext, shader, yk

cd "$(dirname "$0")"

for arg in "$@"; do declare $arg='1'; done

[ "$clean" == "1" ] && rm -f yk && rm -f *.o && rm -f *.spv && echo "cleaned" && exit 0

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

[ "$shader" == "1" ] && for file in gpu/*.vert gpu/*.frag; do
    glslc "$file" -o "$(basename "$file").spv"
done && echo "compiled shaders"

[ "$ext" == "1" ] && $cpp_compiler -std=c++17 $build_type -I. -c src/vma_impl.cpp -o vma.o && echo "compiled vma"

[ "$yk" == "1" ] && $c_compiler -Wno-int-conversion -Wno-incompatible-pointer-types -std=c99 -D_GNU_SOURCE $build_type -I. -c src/main.c -o main.o && echo "compiled yk" && $cpp_compiler main.o vma.o -o yk -lm -lglfw && echo "linked yk and vma"