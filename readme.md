<p>
<h1 align="center">Disco Treehouse</h2>
<p align="center">3D Renderer - C - Vulkan - Linux - Win32</p>
<p align="center">
<img width="600"src="mizuho.jpg">
</p>
</p>

## Compile

The main branch will always compile. `Dev` might not compile on windows since I don't develop on that platform, and I don't like booting in with my windows. (I use Arch btw). However, every now and then I boot up windows to see if it still works.

To build, you need the vulkan sdk (headers + lib + shader compiler), and the vma headers installed.

Vulkan 1.2 is used. Dynamic rendering, BDA, Sync 2 and Descriptor indexing is also used.

### Linux
Xlib is required

```bash
./build.sh ext shader
```

This will compile the engine, external libraries and shaders.

For subsequent builds, you can just do 

```shell
./build.sh
```

Ofc, if you modify either the external libraries or shaders, you will have to recompile.

The following commands are also supported - `clang`, `gcc`, `debug`, `release`, `clean`

### Windows
Only MSVC is supported because it is better.

Remember to have your Vulkan SDK path variable set.
Make sure you have your msvc build environment set up.

```
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
```
This should do it.

```bat
build ext shader
```

This will compile the engine, external libraries and shaders.

For subsequent builds, you can just do 

```bat
build
```

Ofc, if you modify either the external libraries or shaders, you will have to recompile.

The following commands are also supported - `debug`, `release`, `clean`