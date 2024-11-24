# valid args: clean, debug, release, clang, gcc, ext, shader, yk

cd "$(dirname "$0")"

for arg in "$@"; do declare $arg='1'; done

debug_build="-g -O0"
release_build="-O2"
c_compiler="clang"
cpp_compiler="clang++"

build_type="$debug_build"

[ "$release" == "1" ] && build_type="$release_build"
[ "$gcc" == "1" ] && c_compiler="gcc" && cpp_compiler="g++"

[ "$clean" == "1" ] && rm -rf out && echo "cleaned"

mkdir -p out

echo "[$c_compiler]"
echo "[$cpp_compiler]"
echo "[$build_type]"

[ "$shader" == "1" ] && for file in src/*.vert src/*.frag; do
    glslc "$file" -o "out/$(basename "$file").spv"
done && echo "compiled shaders"

[ "$ext" == "1" ] && $cpp_compiler -std=c++17 $build_type -I. -c src/vma_impl.cpp -o out/vma.o && echo "compiled vma"

[ "$yk" == "1" ] && $c_compiler -Wno-int-conversion -Wno-incompatible-pointer-types -std=c99 -D_GNU_SOURCE $build_type -Ilib -c src/main.c -o out/main.o && echo "compiled yk" && $cpp_compiler out/main.o out/vma.o -o out/yk -lm -lglfw && echo "linked yk and vma"