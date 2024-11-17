<p>
<h1 align="center">Ladybird</h2>
<p align="center">3D Renderer - C - Vulkan - Linux - Win32</p>
<p align="center">
<img width="600"src="window.jpg">
</p>
</p>

## Compile

The main branch will always compile. `Dev` might not compile on windows / mac since I don't develop on that platform. I use Arch btw. However, every now and then I boot up windows to see if it still works.

#### Dependencies

- vulkan sdk  ((vk headers + lib + shader compiler + vma headers)

- glfw



Vulkan 1.2 is used. Dynamic rendering, BDA, Sync 2 and Descriptor indexing is also used. I have been used.

### Linux / Mac / Win32

```bash
./build.sh ext shader
```

This will compile the engine, external libraries and shaders.

For subsequent builds, you can just do 

```shell
./build.sh
```

Ofc, if you modify either the external libraries or shaders, you will have to pass `ext` and / or `shader`



The following args are also supported: `shader`,  `debug` , `release`, `clean`



#### Notes

#### Linux  / Mac

 `gcc` and `clang` are also supported as args for the build script. `gcc` is default.



#### Windows

Only msvc is supported because it is better.

Remember to have your Vulkan SDK path variable set.
Make sure you have your msvc build environment set up.

```
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
```

This should do it. You need the x64 developer tools terminal. Powershell won't work unless you've set it up / know what you're doing.