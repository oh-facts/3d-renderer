typedef struct TC_Node TC_Node;
struct TC_Node
{
	TC_Node *next;
	TC_Node *prev;
	u128 hash;
	R_Handle texture;
	u32 scope_ref_count;
};

typedef struct TC_Slot TC_Slot;
struct TC_Slot
{
	TC_Node *first;
	TC_Node *last;
};

typedef struct TC_Touch TC_Touch;
struct TC_Touch
{
	TC_Touch *next;
	u128 hash;
};

typedef struct TC_Scope TC_Scope;
struct TC_Scope
{
	TC_Scope *next;
	TC_Touch *top_touch;
};

typedef struct TC_State TC_State;
struct TC_State 
{
	Arena *arena;
	TC_Slot *slots;
	u64 slot_count;
	
	TC_Node *free_nodes;
	TC_Scope *free_scope;
	TC_Touch *free_touch;
};

global TC_State *tc_state;

function void tc_init()
{
	Arena *arena = arenaAlloc();
	tc_state = pushArray(arena, HS_State, 1);
	tc_state->arena = arena;
	
	tc_state->slot_count = 1024;
	tc_state->slots = pushArray(tc_state->arena, TC_Slot, tc_state->slot_count);
}

function u128 tc_submit(u128 key, R_Handle handle)
{
	
}