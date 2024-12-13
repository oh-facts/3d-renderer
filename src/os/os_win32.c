function void os_init()
{
	Arena *arena = arenaAlloc();
	os_state = pushArray(arena, OS_State, 1);
	os_state->arena = arena;

	char buffer[256];
	DWORD len = GetModuleFileName(0, buffer, 256);
	os_state->file.len = len;
	os_state->file.c = pushArray(arena, u8, len);
		
	out.len = len;
	out.c = pushArray(arena, u8, out.len);
	memcpy(out.c, buffer, out.len);
	pushArray(arena, u8, 1);
	
	Str8function Str8 fileNameFromPath(Arena *arena, Str8 path)


	return out;

}

function void *os_reserve(u64 size)
{
	void *out = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
	if (out != NULL)
	{
		os_state->total_res += size;
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
	os_state->total_cmt += size;
	
	return 1;
}

function void os_decommit(void *ptr, u64 size)
{
	os_state->total_cmt -= size;
	VirtualFree(ptr, size, MEM_DECOMMIT);
}

function void os_free(void *ptr, u64 size)
{
	os_state->total_cmt -= size;
	os_state->total_res -= size;
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