// ok make an os layer
// make a "native" layer
// os layer will also store global commit and res, and increment those
// native layer will abstract over
// so native_getAppDir will get the app dir
// os_getAppDir will return the app dir from os_state
// native does no error checking or anything at all
// os layer does more
// this is going to be alot of work and I don't even know if its worth
// so for now I will just use a static variable that stores app dir

// ok i am getting annoyed

// make the native layer.
// then make the os layer

// so, native_readFile
// native_openFile
// native_sleep
// native_getAppDir
// native_reserve
// and the os layer abstracts over this and does its own shit
// this is the way
// might call platform

typedef struct OS_Handle OS_Handle;
struct OS_Handle
{
	u64 u64[1];
};

typedef struct OS_State OS_State
struct OS_State
{
	struct Arena *arena;
	struct Str8 dir;
	struct Str8 file;
	u64 total_cmt;
	u64 total_res;
};

global OS_State *os_state;

// os hooks ================================== 
// memory
function void os_init();
function void *os_reserve(u64 size);
function b32 os_commit(void *ptr, u64 size);
function void os_decommit(void *ptr, u64 size);
function void os_free(void *ptr, u64 size);
function u64 os_getPageSize();
function void os_sleep(s32 ms);

// proc
function u64 os_getPerfCounter();
function u64 os_getPerfFreq();
function struct FileData os_readFile(struct Arena *arena);

// dll
function OS_Handle os_loadLibrary(char *name);
function void *os_loadFunction(OS_Handle handle, char *name);
// ==================================