//hash and store textures

typedef struct TC_Node TC_Node;
struct TC_Node
{
	TC_Node *next;
	TC_Node *prev;
	u64 hash;
	R_Handle handle;
};

typedef struct TC_Slot TC_Slot;
struct TC_Slot
{
	TC_Slot *first;
	TC_Slot *last;
};

typedef struct TC_State TC_State;
struct TC_State
{
	Arena *arena;
	TC_Slot *slots;
	u64 slot_count;
};