// TODO(mizu): delete queue.
// when something is deleted, its added to a delete queue, that deletes after commands are done executing.
// also i think tex scope isn't freed properly.
// also, work on releasing keys
// then work on asynchronous creation and freeing

typedef struct TEX_KeyNode TEX_KeyNode;
struct TEX_KeyNode
{
	TEX_KeyNode *next;
	TEX_KeyNode *prev;
	u128 key;
	u128 hash;
};

typedef struct TEX_KeySlot TEX_KeySlot;
struct TEX_KeySlot
{
	TEX_KeyNode *first;
	TEX_KeyNode *last;
};

typedef struct TEX_Node TEX_Node;
struct TEX_Node
{
	TEX_Node *next;
	TEX_Node *prev;
	u128 hash;
	R_Handle v;
	Str8 data;
	Arena *data_arena;
	u32 scope_ref_count;
	u32 key_ref_count;
	b32 loaded;
	u64 last_touched_tick;
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
	
	TEX_KeySlot *key_slots;
	u64 key_slot_count;
	
	TEX_Node *free_nodes;
	TEX_KeyNode *free_key_nodes;
	TEX_Scope *free_scope;
	TEX_Touch *free_touch;
	
	u64 ticks;
};

global TEX_State *tex_state;

function void tex_clock_tick()
{
	tex_state->ticks+=1;
}

function void tex_init()
{
	Arena *arena = arenaAlloc();
	tex_state = pushArray(arena, TEX_State, 1);
	tex_state->arena = arena;
	
	tex_state->slot_count = 1024;
	tex_state->slots = pushArray(tex_state->arena, TEX_Slot, tex_state->slot_count);
	
	tex_state->key_slot_count = 1024;
	tex_state->key_slots = pushArray(tex_state->arena, TEX_KeySlot, tex_state->key_slot_count);
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

function u128 tex_hashFromKeyData(u128 key, Str8 data, Arena *data_arena)
{
	u128 hash = tex_hash(data);
	u64 slot_index = hash.u64[1] % tex_state->slot_count;
	TEX_Slot *slot = tex_state->slots + slot_index;
	
	TEX_Node *node = 0;
	
	for(TEX_Node *it = slot->first; it; it = it->next)
	{
		if(u128_equals(it->hash, hash))
		{
			node = it;
		}
	}
	
	// duplicate load
	if(node)
	{
		arenaFree(node->data_arena);
	}
	// add to store
	else
	{
		node = tex_state->free_nodes;
		
		if(node)
		{
			tex_state->free_nodes = tex_state->free_nodes->next;
			*node = (TEX_Node){0};
		}
		else
		{
			node = pushArray(tex_state->arena, TEX_Node, 1);
		}
		
		node->hash = hash;
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
	
	node->key_ref_count += 1;
	
	// store key
	{
		u64 key_slot_index = key.u64[1] % tex_state->key_slot_count;
		
		TEX_KeySlot *key_slot = tex_state->key_slots + key_slot_index;
		
		TEX_KeyNode *key_node = 0;
		for(TEX_KeyNode *cur = key_slot->first; cur; cur = cur->next)
		{
			if(u128_equals(cur->key, key))
			{
				key_node = cur;
				break;
			}
		}
		
		if(!key_node)
		{
			key_node = tex_state->free_key_nodes;
			if(key_node)
			{
				tex_state->free_key_nodes = tex_state->free_key_nodes->next;
			}
			else
			{
				key_node = pushArray(tex_state->arena, TEX_KeyNode, 1);
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

function u128 tex_hashFromkey(TEX_Scope *scope, u128 key)
{
	u128 out = {0};
	
	u64 key_slot_index = key.u64[1] % tex_state->key_slot_count;
	TEX_KeySlot *key_slot = tex_state->key_slots + key_slot_index;
	
	for(TEX_KeyNode *cur = key_slot->first; cur; cur = cur->next)
	{
		if(u128_equals(cur->key, key))
		{
			out = cur->hash;
			break;
		}
	}
	return out;
}

function void tex_evict()
{
	for(u32 slot_index = 0; slot_index < tex_state->slot_count; slot_index++)
	{
		TEX_Slot *slot = tex_state->slots + slot_index;
		for(TEX_Node *it = slot->first; it; it = it->next)
		{
			if((it->scope_ref_count == 0) && (it->loaded == 1) && (it->last_touched_tick + 1 < tex_state->ticks))
			{
				R_VULKAN_Image *image = it->v.u64[0];
				r_vulkan_freeImage(image);
				
				it->loaded = 0;
				
#if 0
				if(it->prev)
				{
					it->prev->next = it->next;
				}
				else
				{
					slot->first = it->next;
				}
				
				if (it->next)
				{
					it->next->prev = it->prev;
				}
				else
				{
					slot->last = it->prev;
				}
				
				it->next = it->prev = 0;
				
				it->next = tex_state->free_nodes;
				tex_state->free_nodes = it;
#endif
				
				break;
			}
		}
	}
}

function R_Handle tex_handleFromHash(TEX_Scope *scope, u128 hash)
{
	R_Handle out = {0};
	u64 slot_index = hash.u64[1] % tex_state->slot_count;
	TEX_Slot *slot = tex_state->slots + slot_index;
	
	TEX_Node *node = 0;
	
	for(TEX_Node *it = slot->first; it; it = it->next)
	{
		if(u128_equals(it->hash, hash))
		{
			node = it;
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
	
	Assert(node);
	
	if(!node->loaded)
	{
		// allocate texture
		Bitmap bmp = *(Bitmap*)node->data.c;
		R_VULKAN_Image *image = r_vulkan_image(bmp);
		node->v.u64[0] = image;
		node->loaded = 1;
	}
	
	node->last_touched_tick = tex_state->ticks;
	out = node->v;
	
	return out;
}