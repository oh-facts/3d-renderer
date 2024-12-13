<p>
<h1 align="center">Ladybird</h2>
<p align="center">3D Vulkan Renderer</p>
<p align="center">
<img width="300"src="res/ladybird/ladybird.png">
</p>
</p>

Vulkan research renderer. I am exploring bindless rendering, asset streaming, gltf materials and global illumination. End goal is to learn more about modern gpus and gfx programming. I might make a game with it for fun.

<p>
<h1 align="center">Screenshots</h2>
<p align="center">
<img width="400"src="res/ladybird/demo2.png">
<img width="400"src="res/ladybird/demo.png">
</p>
</p>

## 3d renderer demo
- Entry point inside `src/ladybird/`
- `wasd` to move, mouse to look
- `esc` to toggle control

## Compile

Vulkan 1.2 is used. Dynamic rendering, BDA, Sync 2 and Descriptor indexing is also used. I have been used.

The main branch will always compile on windows, linux and mac. 

`Dev` might not compile on windows / mac since I use Arch btw. However, every now and then, I boot up windows to make it compile.

`hot` might only compile on my machine. Sometimes it doesn't, but I still push it. Git is "free" cloud storage

Mac build and vulkan support has been authored by @gruelingpine185

#### Dependencies

- vulkan sdk (vk headers and lib + vma headers + glslc)

- glfw (For win32, binaries and headers are shipped with the repo)

- C99 and C++17 compiler

- git-lfs (for storing resources)

Dependencies not listed here are shipped with the project in `lib`

```
git clone https://github.com/oh-facts/3d-renderer.git
cd 3d-renderer
git lfs pull
```

You will need to install `git-lfs`.

### Linux

```shell
./build.sh ext shader yk
```

This will compile the external libraries, the shaders and the renderer with clang in debug mode.

The following args are also supported:  `debug` , `release`, `clean`, `clang`, `gcc`

### Mac

```

```

### Windows

```batch
build vma shader ladybird
```

This will compile vma, the shaders and the renderer demo with msvc in debug mode.

The following args are also supported: `debug` , `release`, `clean`

Remember to have your Vulkan SDK path variable set.
Make sure you have your msvc build environment set up.

```
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
```

This should do it. You need the x64 developer tools terminal. Powershell won't work unless you've set it up / know what you're doing.

### Contributions
Open Issue. Discuss Issue. Open PR.

### Project Structure

All source code is inside `src/`
- `base/`: common utility functions. Arenas, basic types, file handling - bitmaps, file descriptors, font / gltf loading, math functions and string abstraction
- `gltf/`: gltf loader.
- `ladybird/`: 3d renderer testbed
- `os/` : os abstraction. win32, glfw and unix.
- `render/` : renderer layer. Used to draw rect2d, rect3d and meshes. Currently only supports a vulkan 1.2 backend
- `shaders/` : glsl shaders, used by the vulkan renderer backend
- `texture/` : texture cache

Third party libraries inside `lib/`
- `blake2`: For hashing when requirements demand for unique hashes
- `cgltf`: gltf parser
- `GLFW` : cross platform windowing library 
- `stb` : Sean Barrett's image, ttf and sprintf
- `vma` : vulkan memory allocator

Resources inside `res/`, tracked using git-lfs