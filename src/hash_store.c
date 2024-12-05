typedef struct HS_KeyNode HS_KeyNode;
struct HS_KeyNode
{
	HS_KeyNode *next;
	HS_KeyNode *prev;
	u128 key;
	u128 hash;
};

typedef struct HS_KeySlot HS_KeySlot;
struct HS_KeySlot
{
	HS_KeyNode *first;
	HS_KeyNode *last;
};

typedef struct HS_Node HS_Node;
struct HS_Node
{
	HS_Node *next;
	HS_Node *prev;
	u128 hash;
	Str8 data;
	u32 key_ref_count;
	u32 scope_ref_count;
};

typedef struct HS_Slot HS_Slot;
struct HS_Slot
{
	HS_Node *first;
	HS_Node *last;
};

typedef struct HS_Touch HS_Touch;
struct HS_Touch
{
	HS_Touch *next;
	u128 hash;
};

typedef struct HS_Scope HS_Scope;
struct HS_Scope
{
	HS_Scope *next;
	HS_Touch *top_touch;
};

typedef struct HS_State HS_State;
struct HS_State 
{
	Arena *arena;
	HS_Slot *slots;
	u64 slot_count;
	
	HS_KeySlot *key_slots;
	u64 key_slot_count;
	
	HS_Node *free_nodes;
	HS_KeyNode *free_key_nodes;
	
	HS_Scope *free_scope;
	HS_Touch *free_touch;
};

global HS_State *hs_state;

function void hs_init()
{
	Arena *arena = arenaAlloc();
	hs_state = pushArray(arena, HS_State, 1);
	hs_state->arena = arena;
	
	hs_state->slot_count = 1024;
	hs_state->slots = pushArray(hs_state->arena, HS_Slot, hs_state->slot_count);
	
	hs_state->key_slot_count = 1024;
	hs_state->key_slots = pushArray(hs_state->arena, HS_KeySlot, hs_state->key_slot_count);
}

function u128 hs_hash(Str8 str)
{
	u128 u128 = {0};
  blake2b((u8 *)&u128.u64[0], sizeof(u128), str.c, str.len, 0, 0);
  return u128;
}

function u128 hs_submit(u128 key, Str8 data)
{
	u128 hash = hs_hash(data);
	u64 slot_index = hash.u64[1] % hs_state->slot_count;
	
	HS_Slot *slot = hs_state->slots + slot_index;
	
	u64 key_slot_index = key.u64[1] % hs_state->key_slot_count;
	
	HS_KeySlot *key_slot = hs_state->key_slots + key_slot_index;
	
	// add to cache. if its present, increase ref count
	{
		HS_Node *exists = 0;
		
		for(HS_Node *cur = slot->first; cur; cur = cur->next)
		{
			if(u128_equals(cur->hash, hash))
			{
				exists = cur;
			}
		}
		
		if(!exists)
		{
			HS_Node *node = hs_state->free_nodes;
			if(node)
			{
				hs_state->free_nodes = hs_state->free_nodes->next;
			}
			else
			{
				node = pushArray(hs_state->arena, HS_Node, 1);
			}
			
			node->hash = hash;
			node->key_ref_count = 1;
			node->data = data;
			
			if(!slot->first)
			{
				slot->first = slot->last = node;
			}
			else
			{
				node->prev = slot->last;
				slot->last->next = node;
				slot->last = node;
			}
		}
		else
		{
			exists->key_ref_count += 1;
		}
	}
	
	// store key
	{
		HS_KeyNode *key_node = 0;
		for(HS_KeyNode *cur = key_slot->first; cur; cur = cur->next)
		{
			if(u128_equals(cur->key, key))
			{
				key_node = cur;
				break;
			}
		}
		
		if(!key_node)
		{
			key_node = hs_state->free_key_nodes;
			if(key_node)
			{
				hs_state->free_key_nodes = hs_state->free_key_nodes->next;
			}
			else
			{
				key_node = pushArray(hs_state->arena, HS_KeyNode, 1);
			}
			
			key_node->key = key;
			
			if(!key_slot->first)
			{
				key_slot->first = key_slot->last = key_node;
			}
			else
			{
				key_node->prev = key_slot->last;
				key_slot->last->next = key_node;
				key_slot->last = key_node;
			}
		}
		key_node->hash = hash;
	}
	
	return hash;
}

function u128 hs_hashFromKey(u128 key)
{
	u128 out = {0};
	
	u64 key_slot_index = key.u64[1] % hs_state->slot_count;
	HS_KeySlot *key_slot = hs_state->key_slots + key_slot_index;
	
	for(HS_KeyNode *cur = key_slot->first; cur; cur = cur->next)
	{
		if(u128_equals(cur->key, key))
		{
			out = cur->hash;
			break;
		}
	}
	return out;
}

function Str8 hs_dataFromHash(HS_Scope *scope, u128 hash)
{
	Str8 out = {0};
	u64 slot_index = hash.u64[1] % hs_state->slot_count;
	HS_Slot *slot = hs_state->slots + slot_index;
	
	for(HS_Node *cur = slot->first; cur; cur = cur->next)
	{
		if(u128_equals(cur->hash, hash))
		{
			out = cur->data;
			cur->scope_ref_count+=1;
			
			HS_Touch *touch = hs_state->free_touch;
			
			if(!touch)
			{
				touch = pushArray(hs_state->arena, HS_Touch, 1);
			}
			else
			{
				hs_state->free_touch = hs_state->free_touch->next;
			}
			*touch = (HS_Touch){0};
			touch->hash = cur->hash;
			touch->next = scope->top_touch;
			scope->top_touch = touch;
			break;
		}
	}
	return out;
}

function HS_Scope *hs_scopeOpen()
{
	HS_Scope *out = hs_state->free_scope;
	
	if(!out)
	{
		out = pushArray(hs_state->arena, HS_Scope, 1);
	}
	else
	{
		hs_state->free_scope = hs_state->free_scope->next;
	}
	*out = (HS_Scope){0};
	
	return out;
}

function void hs_scopeClose(HS_Scope *scope)
{
	for(HS_Touch *touch = scope->top_touch, *next = 0; touch; touch = next)
	{
		u128 hash = touch->hash;
		next = touch->next;
		
		u64 slot_index = hash.u64[1] % hs_state->slot_count;
		HS_Slot *slot = hs_state->slots + slot_index;
		
		for(HS_Node *node = slot->first; node; node = node->next)
		{
			if(u128_equals(node->hash, hash))
			{
				node->scope_ref_count-=1;
				break;
			}
		}
		touch->next = hs_state->free_touch;
		hs_state->free_touch = touch;
	}
	scope->next = hs_state->free_scope;
	hs_state->free_scope = scope;
}