typedef struct R_Handle R_Handle;
struct R_Handle
{
	u64 u64[2];
};

typedef struct R_Rect3 R_Rect3;
struct R_Rect3
{
	M4F model;
	u32 tex_id;
	u32 pad[3];
};

typedef struct R_Batch R_Batch;
struct R_Batch
{
	u8 *base;
	u32 num;
	u32 size;
	u32 cap;
};

function void r_pushRect3(R_Batch *batch, M4F model, u32 tex_id)
{
	Assert(batch->base + batch->cap > batch->size + sizeof(R_Rect3));
	R_Rect3 *rect3 = batch->base + batch->size;
	rect3->model = model;
	rect3->tex_id = tex_id;
	
	batch->num+=1;
	batch->size+=sizeof(R_Rect3);
}