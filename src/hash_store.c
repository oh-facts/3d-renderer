typedef struct HS_Node HS_Node;
struct HS_Node
{
	HS_Node *next;
	HS_Node *prev;
	u64 hash;
	Str8 data;
};

typedef struct HS_NodeSlot HS_NodeSlot;
struct HS_NodeSlot
{
	HS_NodeSlot *next;
	HS_NodeSlot *prev;
	u64 hash;
	Str8 data;
};

typedef struct HS_State HS_State
{
	Arena *arena;
	HS_NodeSlot *slots;
	u64 slot_count;
};