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

typedef struct R_Rect3 R_Rect3;
struct R_Rect3
{
	M4F model;
	u32 tex_id;
	u32 pad[3];
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

typedef struct R_Batch R_Batch;
struct R_Batch
{
	u8 *base;
	u32 num;
	u32 size;
	u32 cap;
};

typedef struct R_State R_State;
struct R_State
{
	s32 temp;
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