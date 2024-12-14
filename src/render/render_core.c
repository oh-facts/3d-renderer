#define Corner_00 0
#define Corner_01 1
#define Corner_10 2
#define Corner_11 3
#define Corner_COUNT 4

typedef struct R_Handle R_Handle;
struct R_Handle
{
	u64 u64[2];
};

typedef struct R_Rect2 R_Rect2;
struct R_Rect2
{
	RectF32 src;
	RectF32 dst;
	V4F border_color;
	V4F fade[Corner_COUNT];
	u32 tex_id;
	u32 pad[3];
	f32 border_thickness;
	f32 radius;
	f32 pad2[2];
};

typedef struct R_Rect3 R_Rect3;
struct R_Rect3
{
	M4F model;
	u32 tex_id;
	u32 pad[3];
};

typedef struct R_Vertex R_Vertex;
struct R_Vertex
{
	V3F pos;
	f32 uv_x;
	V3F normal;
	f32 uv_y;
	V4F color;
	V3F tangent;
	float pad;
};

// I am thinking - begin indexed call
// end indexed call
// For future me, I was thinking - push buffer styled rendering
// also, not everything needs to be a ptr (speaking of buffers, etc)
// I can either a) put identifying info inside a handle
// b) for stuff that needs to / could be in a queue, I can use meta nodes
// Its weird making everything a ptr.
// First, make this stuff compile. make everything a ptr for testing purposes.
// Then see if you can make textures work as not ptrs.
// This is what I am thinking
// buffer and images are created as normal stack objects
// When you do handleFromBuffer or handleFromTexture, you received heap
// based meta nodes
typedef struct R_BindBuffer R_BindBuffer;
struct R_BindBuffer
{
	u32 num_primitives;
	R_Handle mesh_buffer;
};

typedef struct R_DrawIndexed R_DrawIndexed;
struct R_DrawIndexed
{
	u32 start;
	u32 count;
	R_Handle base_tex;
	R_Handle normal_tex;
	M4F transform;
};

typedef struct R_Batch R_Batch;
struct R_Batch
{
	u8 *base;
	u32 num;
	u32 size;
	u32 cap;
};

function R_Rect2 *r_pushRect2(R_Batch *batch, RectF32 dst, V4F color)
{
	Assert(batch->base + batch->cap > batch->size + sizeof(R_Rect2));
	
	R_Rect2 *out = batch->base + batch->size;
	out->dst = dst;
	out->src = rectF32(0, 0, 1, 1);
	out->fade[Corner_00] = color;
	out->fade[Corner_01] = color;
	out->fade[Corner_10] = color;
	out->fade[Corner_11] = color;
	out->tex_id = 0;
	
	// NOTE(mizu): figure out why there is a 1px ghost outline even when thickness is 0
	out->border_color = color;
	out->radius = 0;
	out->border_thickness = 0;
	
	batch->num+=1;
	batch->size+=sizeof(R_Rect2);
	
	return out;
}

function R_Rect3 *r_pushRect3(R_Batch *batch, M4F model, u32 tex_id)
{
	Assert(batch->base + batch->cap > batch->size + sizeof(R_Rect3));
	R_Rect3 *out = batch->base + batch->size;
	out->model = model;
	out->tex_id = tex_id;
	
	batch->num+=1;
	batch->size+=sizeof(R_Rect3);
	
	return out;
}

function void r_beginMesh(R_Batch *batch, u32 num_primitives, R_Handle mesh_buffer)
{
	Assert(batch->base + batch->cap > batch->size + sizeof(R_BindBuffer));
	R_BindBuffer *bind_index = batch->base + batch->size;
	bind_index->num_primitives = num_primitives;
	bind_index->mesh_buffer = mesh_buffer;

	batch->num+=1;
	batch->size+= sizeof(R_BindBuffer);
}

function void r_pushMesh(R_Batch *batch, u32 start, u32 count, R_Handle base_tex, R_Handle normal_tex, M4F transform)
{
	Assert(batch->base + batch->cap > batch->size + sizeof(R_DrawIndexed));
	R_DrawIndexed *draw = batch->base + batch->size;
	draw->start = start;
	draw->count = count;
	draw->base_tex = base_tex;
	draw->normal_tex = normal_tex;
	draw->transform = transform;

	batch->size+= sizeof(R_DrawIndexed);	
}