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
    
    void *memory = os_reserve(res);
    os_commit(memory, cmt);
    
    arena = (Arena*)memory;
    arena->used = ARENA_HEADER_SIZE;
    arena->align = DEFAULT_ALIGN;
    
    arena->cmt = alignPow2(cmt, os_getPageSize());
    arena->res = res;
    
    return arena;
}

function Arena *arenaAlloc()
{
    return arenaAllocSized(ARENA_COMMIT_SIZE, ARENA_RESERVE_SIZE);
}

typedef enum DEBUG_CYCLE_COUNTER DEBUG_CYCLE_COUNTER;
enum DEBUG_CYCLE_COUNTER
{
    DEBUG_CYCLE_COUNTER_UPDATE_AND_RENDER,
    DEBUG_CYCLE_COUNTER_COUNT
};

read_only char *cycle_to_str[DEBUG_CYCLE_COUNTER_COUNT] =
{
    "update & render",
};

typedef struct cycleCounter cycleCounter;
struct cycleCounter
{
    u64 cycle_count;
    u32 hit_count;
};

typedef struct TCXT TCXT;
struct TCXT
{
    Arena *arena;
    Arena *arenas[2];
    cycleCounter counters[DEBUG_CYCLE_COUNTER_COUNT];
    cycleCounter counters_last[DEBUG_CYCLE_COUNTER_COUNT];
};

global TCXT *tcxt;

#if defined (OS_WIN32) || defined(OS_LINUX)
#define BEGIN_TIMED_BLOCK(ID) u64 start_cycle_count_##ID = __rdtsc(); ++tcxt->counters[DEBUG_CYCLE_COUNTER_##ID].hit_count
#define END_TIMED_BLOCK(ID)  tcxt->counters[DEBUG_CYCLE_COUNTER_##ID].cycle_count += __rdtsc() - start_cycle_count_##ID

#else
#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#endif

function void tcxt_init()
{
    Arena *arena = arenaAlloc();
    tcxt = pushArray(arena, TCXT, 1);
    tcxt->arena = arena;
    for(u32 i = 0; i < arrayLen(tcxt->arenas); i ++)
    {
        tcxt->arenas[i] = arenaAllocSized(MB(10), MB(64));
    }
}

function void tcxt_process_debug_counters()
{
    for(u32 i = 0; i < arrayLen(tcxt->counters); i ++)
    {
        cycleCounter *counter = tcxt->counters + i;
        cycleCounter *counter_last = tcxt->counters_last + i;
        
        counter_last->hit_count = counter->hit_count;
        counter_last->cycle_count = counter->cycle_count;
        
        //printf("%d: %lu\n", i, counter->cycle_count);
        counter->hit_count = 0;
        counter->cycle_count = 0;
    }
}

function void tcxt_print_debug_counters()
{
    for(u32 i = 0; i < arrayLen(tcxt->counters); i ++)
    {
        cycleCounter *counter = tcxt->counters_last + i;
        
        printf("%s: %lu\n", cycle_to_str[i], counter->cycle_count);
    }
}

function Arena *tcxt_get_scratch(Arena **conflicts, u64 count)
{
    Arena *out = 0;
    for(u32 i = 0; i < arrayLen(tcxt->arenas); i ++)
    {
        b32 has_conflict = 0;
        for(u32 j = 0; j < count; j ++)
        {
            if(tcxt->arenas[i] == conflicts[j])
            {
                has_conflict = 1;
                break;
            }
        }
        if(!has_conflict)
        {
            out = tcxt->arenas[i];
        }
    }
    
    return out;
}

#define scratch_begin(conflicts, count) arenaTempBegin(tcxt_get_scratch(conflicts, count))
#define scratch_end(scratch) arenaTempEnd(scratch);