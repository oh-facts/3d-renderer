/* date = August 6th 2024 10:51 pm */

#if defined(_WIN32)
#define OS_WIN32
#elif defined (__linux__)
#define OS_LINUX
#elif defined(__APPLE__)
#define OS_APPLE
#else
#error This OS is not supported
#endif

#if defined(__clang__)
#define COMPILER_CLANG
#elif defined(_MSC_VER)
#define COMPILER_MSVC
#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER_GCC
#else
#error This compiler is not supported
#endif

#if defined(COMPILER_CLANG)
#define TRAP() __builtin_trap()
#elif defined(COMPILER_MSVC)
#define TRAP() __debugbreak()
#elif defined(COMPILER_GCC)
#define TRAP() __builtin_trap()
#endif

#define DEFAULT_ALIGN sizeof(void *)
#define ENABLE_ASSERTS 1

#define _Assert_helper(expr) \
do                           \
{                            \
if (!(expr))                 \
{                            \
TRAP();                      \
}                            \
} while (0)

#if ENABLE_ASSERTS
#define Assert(expr) _Assert_helper(expr)
#else
#define Assert(expr)
#endif

#define AssertAlways(expr) _Assert_helper(expr)

#define INVALID_CODE_PATH() _Assert_helper(0)

#define NOT_IMPLEMENTED() _Assert_helper(0)

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef int32_t b32;

#define KB(Value) ((u64)(Value) * 1024)
#define MB(Value) (KB(Value) * 1024)
#define GB(Value) (MB(Value) * 1024)
#define TB(Value) (GB(Value) * 1024)

#define PI (3.1415926535897f)
#define degToRad(deg) (deg * PI / 180.f)
#define alignPow2(x,b) (((x) + (b) - 1)&(~((b) - 1)))
#define clampTop(A,X) min(A,X)
#define clampBot(X,B) max(X,B)

#define addFlag(flags, flagToAdd) ((flags) |= (flagToAdd))
#define removeFlag(flags, flagToRemove) ((flags) &= ~(flagToRemove))
#define isFlagSet(flags, flagToCheck) ((flags) & (flagToCheck))
#define toggleFlag(flags, flagToToggle) ((flags) ^= (flagToToggle))
#define arrayLen(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define deferLoop(begin, end) for(int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))

#define global static
#define function static

#define local_persist static
#define read_only static const

#if defined OS_WIN32
#define export_function __declspec(dllexport)
#else
#define export_function __attribute__((visibility("default")))
#endif

#if defined(OS_WIN32)
#define _CRT_SECURE_NO_WARNINGS

// unused function
#pragma warning(disable: 4505)

// nameless structs
#pragma warning(disable: 4201)

// loss of data
#pragma warning(disable: 4244)

// i dont remember
/*
#pragma warning(disable: 4576)
#pragma warning(disable: 4456)
#pragma warning(disable: 4305)
#pragma warning(disable: 4018)
*/
#endif

#define STB_SPRINTF_IMPLEMENTATION
#include <stb/stb_sprintf.c>

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.c>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.c>

#include <stdio.h>
#include <math.h>

#include "os_core.c"
#include "base_core.c"
#include "base_math.c"
#include "base_string.c"
#include "base_file.c"

#include "os_gfx.c"

#undef function
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#define function static
#include "os_vulkan.c"

#if defined(OS_WIN32)
#undef function
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define function static
#include "os_win32.c"
#elif defined(OS_LINUX)
#undef function
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <time.h>
#define function static
#include "os_unix.c"
#elif defined(OS_APPLE)
#undef function
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <time.h>
#define function static
#include "os_unix.c"
#else
#error platform not supported
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "os_glfw.c"

#include "render_vulkan.c"