global u64 total_cmt;
global u64 total_res;
typedef struct OS_Handle OS_Handle;
struct OS_Handle
{
	u64 u64[1];
};

// memory
function void *os_reserve(u64 size);
function b32 os_commit(void *ptr, u64 size);
function void os_decommit(void *ptr, u64 size);
function void os_free(void *ptr, u64 size);
function u64 os_getPageSize();
function void os_sleep(s32 ms);

// proc
struct Arena;
function struct Str8 os_getAppDir(struct Arena *arena);
function u64 os_getPerfCounter();
function u64 os_getPerfFreq();

// dll
function OS_Handle os_loadLibrary(char *name);
function void *os_loadFunction(OS_Handle handle, char *name);