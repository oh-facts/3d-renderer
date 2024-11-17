function void *os_reserve(u64 size)
{
	void *out = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	total_res += size;
	return out;
}

function b32 os_commit(void *ptr, u64 size)
{
	if(mprotect(ptr, size, PROT_READ | PROT_WRITE) == -1)
	{
		int err = errno;
		printf("mprotect failed: %s\r\n", strerror(err));
		return 0;
	}
	total_cmt += size;
	return 1;
}

function void os_decommit(void *ptr, u64 size)
{
	madvise(ptr, size, MADV_DONTNEED);
	mprotect(ptr, size, PROT_NONE);
}

function void os_free(void *ptr, u64 size)
{
	munmap(ptr, size);
}

function u64 os_getPageSize()
{
	return getpagesize();
}

function void os_sleep(s32 ms)
{
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, 0);
}

function Str8 os_getAppDir(Arena *arena)
{
	char buffer[256];
	ssize_t len = readlink("/proc/self/exe", buffer, 256);
	
	char *c = &buffer[len];
	while(*(--c) != '/')
	{
		*c = 0;
		--len;
	}
	
	u8 *str = pushArray(arena, u8, len);
	memcpy(str, buffer, len);
	
	Str8 out = str8(str, len);
	
	return out;
}

function u64 os_getPerfCounter() 
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

function u64 os_getPerfFreq() 
{
	return 1000000000ull;
}

function OS_Handle os_loadLibrary(char *name)
{
	OS_Handle out = {0};
	void *dll = dlopen(name, RTLD_NOW);
	printf("%s %p\r\n", name, dll);
	out.u64[0] = (uint64_t)dll;
	return out;
}

function void *os_loadFunction(OS_Handle handle, char *name)
{
	void *dll = (void *)handle.u64[0];
	void *out = dlsym(dll, name);
	return out;
}

function OS_Handle os_vulkan_loadLibrary()
{
#if defined(OS_LINUX)
    
    OS_Handle out = os_loadLibrary("libvulkan.so.1"); 
    
    return out;
    
#elif defined(OS_APPLE)
    
    OS_Handle out = os_loadLibrary("/usr/local/lib/libvulkan.dylib");
	
    return out;
    
#endif
}