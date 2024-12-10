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

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct u128 u128;
struct u128
{
	u64 u64[2];
};

static int32_t u128_equals(u128 a, u128 b)
{
	return (a.u64[0] == b.u64[0]) && (a.u64[1] == b.u64[1]);
}

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
#pragma comment (lib, "kernel32")
#pragma comment (lib, "user32")
#pragma comment (lib, "gdi32")
#pragma comment (lib, "shell32")
#else
#define export_function __attribute__((visibility("default")))
#endif

