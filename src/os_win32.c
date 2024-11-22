function void *os_reserve(u64 size)
{
	void *out = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
	if (out != NULL)
	{
		total_res += size;
	}
	return out;
}

function b32 os_commit(void *ptr, u64 size)
{
	if (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) == NULL)
	{
		printf("VirtualAlloc commit failed: %lu\r\n", GetLastError());
		return 0;
	}
	total_cmt += size;
	
	return 1;
}

function void os_decommit(void *ptr, u64 size)
{
	VirtualFree(ptr, size, MEM_DECOMMIT);
}

function void os_free(void *ptr, u64 size)
{
	VirtualFree(ptr, 0, MEM_RELEASE);
}

function u64 os_getPageSize()
{
	SYSTEM_INFO sysinfo = {0};
	GetSystemInfo(&sysinfo);
	return sysinfo.dwPageSize;
}

function void os_sleep(s32 ms)
{
	Sleep(ms);
}

function Str8 os_getAppDir(Arena *arena)
{
	char buffer[256];
	DWORD len = GetModuleFileName(0, buffer, 256);
	
	char *c = &buffer[len];
	while(*(--c) != '\\')
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
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return (u64)counter.QuadPart;
}

function u64 os_getPerfFreq()
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return (u64)frequency.QuadPart;
}

function OS_Handle os_loadLibrary(char *name)
{
	OS_Handle out = {0};
	HMODULE dll = LoadLibraryA(name);
	out.u64[0] = (u64)dll;
	return out;
}

function void *os_loadFunction(OS_Handle handle, char *name)
{
	HMODULE dll = (HMODULE)handle.u64[0];
	void *out = (void*)GetProcAddress(dll, name);
	return out;
}

function OS_Handle os_vulkan_loadLibrary()
{
	return os_loadLibrary("vulkan-1.dll");
}