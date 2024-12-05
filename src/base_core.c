function b32 isPow2(size_t addr)
{
	return (addr & (addr-1)) == 0;
}

typedef struct Arena Arena;
struct Arena
{
	Arena *next;
	u64 used;
	u64 align;
	u64 cmt;
	u64 res;
};

#define ARENA_COMMIT_SIZE KB(64)
#define ARENA_RESERVE_SIZE MB(64)
#define ARENA_HEADER_SIZE 128
#define ARENA_ARR_LEN(arena, type) (arena->used / sizeof(type))

#ifndef min
#define min(A,B) (((A)<(B))?(A):(B))
#define max(A,B) (((A)>(B))?(A):(B))
#endif

typedef struct ArenaTemp ArenaTemp;
struct ArenaTemp
{
	Arena *arena;
	u64 pos;
};

#define pushArray(arena,type,count) (type*)arenaPushImpl(arena, sizeof(type) * count)

function void* arenaPushImpl(Arena* arena, size_t size)
{
	u64 pos_mem = alignPow2(arena->used, arena->align);
	u64 pos_new = pos_mem + size;
	
	if(arena->res < pos_new)
	{
		// TODO(mizu): deal with reserving more (chain arenas)
		INVALID_CODE_PATH();
	}
	
	if(arena->cmt < pos_new)
	{
		u64 cmt_new_aligned, cmt_new_clamped, cmt_new_size;
		
		cmt_new_aligned = alignPow2(pos_new, ARENA_COMMIT_SIZE);
		cmt_new_clamped = clampTop(cmt_new_aligned, arena->res);
		cmt_new_size    = cmt_new_clamped - arena->cmt;
		os_commit((u8*)arena + arena->cmt, cmt_new_size);
		arena->cmt = cmt_new_clamped;
	}
	
	void *memory = 0;
	
	if (arena->cmt >= pos_new)
	{
		memory = (u8*)arena + pos_mem;
		arena->used = pos_new;
	}
	
	return memory;
}

function ArenaTemp arenaTempBegin(Arena *arena)
{
	ArenaTemp out = {
		.arena = arena,
		.pos = arena->used,
	};
	return out;
}

// TODO(mizu): Can be very expensive
function void arenaTempEnd(ArenaTemp *temp)
{
	memset((u8*)temp->arena + temp->pos, 0, temp->arena->used - temp->pos);
	
	temp->arena->used = temp->pos;
}

function Arena *arenaAllocSized(u64 cmt, u64 res)
{
	Arena *arena = 0;
	
	u64 page_size = os_getPageSize();
	res = alignPow2(res, page_size);
	cmt = alignPow2(cmt, page_size);
	
	void *memory = os_reserve(res);
	os_commit(memory, cmt);
	
	arena = (Arena*)memory;
	arena->used = ARENA_HEADER_SIZE;
	arena->align = DEFAULT_ALIGN;
	
	arena->cmt = cmt;
	arena->res = res;
	
	return arena;
}

function Arena *arenaAlloc()
{
	return arenaAllocSized(ARENA_COMMIT_SIZE, ARENA_RESERVE_SIZE);
}

function void arenaFree(Arena *arena)
{
	os_free(arena, arena->res);
}