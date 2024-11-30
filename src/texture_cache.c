typedef struct TEX_Node TEX_Node;
struct TEX_Node
{
	TEX_Node *next;
	TEX_Node *prev;
	u128 hash;
	R_Handle v;
	u32 scope_ref_count;
};

typedef struct TEX_Slot TEX_Slot;
struct TEX_Slot
{
	TEX_Node *first;
	TEX_Node *last;
};

typedef struct TEX_Touch TEX_Touch;
struct TEX_Touch
{
	TEX_Touch *next;
	u128 hash;
};

typedef struct TEX_Scope TEX_Scope;
struct TEX_Scope
{
	TEX_Scope *next;
	TEX_Touch *top_touch;
};

typedef struct TEX_State TEX_State;
struct TEX_State 
{
	Arena *arena;
	TEX_Slot *slots;
	u64 slot_count;
	
	TEX_Node *free_nodes;
	
	TEX_Scope *free_scope;
	TEX_Touch *free_touch;
};

global TEX_State *tex_state;

function void tex_init()
{
	Arena *arena = arenaAlloc();
	tex_state = pushArray(arena, TEX_State, 1);
	tex_state->arena = arena;
	
	tex_state->slot_count = 1024;
	tex_state->slots = pushArray(tex_state->arena, TEX_Slot, tex_state->slot_count);
}

function u128 tex_hash(Str8 str)
{
	u128 u128 = {0};
  blake2b((u8 *)&u128.u64[0], sizeof(u128), str.c, str.len, 0, 0);
  return u128;
}

function TEX_Scope *tex_scopeOpen()
{
	TEX_Scope *out = tex_state->free_scope;
	
	if(!out)
	{
		out = pushArray(tex_state->arena, TEX_Scope, 1);
	}
	else
	{
		tex_state->free_scope = tex_state->free_scope->next;
	}
	*out = (TEX_Scope){0};
	
	return out;
}

function void tex_scopeClose(TEX_Scope *scope)
{
	for(TEX_Touch *touch = scope->top_touch, *next = 0; touch; touch = next)
	{
		u128 hash = touch->hash;
		next = touch->next;
		
		u64 slot_index = hash.u64[1] % tex_state->slot_count;
		TEX_Slot *slot = tex_state->slots + slot_index;
		
		for(TEX_Node *node = slot->first; node; node = node->next)
		{
			if(u128_equals(node->hash, hash))
			{
				node->scope_ref_count-=1;
				break;
			}
		}
		touch->next = tex_state->free_touch;
		tex_state->free_touch = touch;
	}
	scope->next = tex_state->free_scope;
	tex_state->free_scope = scope;
}

function u128 tex_hashFromKeyData(TEX_Scope *scope, u128 key, Str8 data)
{
	
}

function u128 tex_hashFromkey(TEX_Scope *scope, u128 key)
{
	
}

function void tex_releaseKey(u128 key)
{
	
}

function R_Handle tex_handleFromHash(TEX_Scope *scope, u128 hash)
{
	R_Handle out = {0};
	u64 slot_index = hash.u64[1] % tex_state->slot_count;
	TEX_Slot *slot = tex_state->slots + slot_index;
	
	for(TEX_Node *it = slot->first; it; it = it->next)
	{
		if(u128_equals(it->hash, hash))
		{
			out = it->v;
			it->scope_ref_count+=1;
			TEX_Touch *touch = tex_state->free_touch;
			
			if(!touch)
			{
				touch = pushArray(tex_state->arena, TEX_Touch, 1);
			}
			else
			{
				tex_state->free_touch = tex_state->free_touch->next;
			}
			*touch = (TEX_Touch){0};
			touch->hash = it->hash;
			touch->next = scope->top_touch;
			scope->top_touch = touch;
			break;
			
		}
	}
	
	if(!out.u64[0])
	{
		
		
	}
	
	return out;
}