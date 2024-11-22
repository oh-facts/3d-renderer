<p>
<h1 align="center">Ladybird</h2>
<p align="center">3D Renderer - C - Vulkan</p>
<p align="center">
<img width="400"src="ladybird.png">
</p>
</p>

## Compile

The main branch will always compile on windows, linux and mac. 

`Dev` might not compile on windows / mac since I use Arch btw. However, every now and then, I boot up windows to make it compile.

Mac build and vulkan support has been authored by @gruelingpine185

Vulkan 1.2 is used. Dynamic rendering, BDA, Sync 2 and Descriptor indexing is also used. I have been used.

#### Dependencies

- vulkan sdk (vk headers + lib + vma headers + glslc)

- glfw

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
build ext shader yk
```

This will compile the external libraries, the shaders and the renderer with clang in debug mode.

The following args are also supported: `debug` , `release`, `clean`

Only msvc is supported because it is better.

Remember to have your Vulkan SDK path variable set.
Make sure you have your msvc build environment set up.

```
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
```

This should do it. You need the x64 developer tools terminal. Powershell won't work unless you've set it up / know what you're doing.