typedef union v2f v2f;
union v2f
{
	f32 e[2];
	struct
	{
		f32 x;
		f32 y;
	};
};

typedef union v2s v2s;
union v2s
{
	s32 e[2];
	struct
	{
		s32 x;
		s32 y;
	};
};

function b32 v2s_equals(v2s a, v2s b)
{
	if((a.x == b.x) && (a.y == b.y))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

typedef struct RectF32 RectF32;
struct RectF32
{
	v2f min;
	v2f max;
};

function RectF32 rectF32(f32 min_x, f32 min_y, f32 max_x, f32 max_y)
{
	RectF32 out = {0};
	
	out.min.x = min_x;
	out.min.y = min_y;
	
	out.max.x = max_x;
	out.max.y = max_y;
	
	return out;
}

#define rect_minVarg(v) (v).min.x, (v).min.y
#define rect_macVarg(v) (v).max.x, (v).max.y
#define rect_varg(v) rect_minVarg(v), rect_macVarg(v)

#define v4f_varg(v) (v).x, (v).y, (v).z, (v).w

function RectF32 rectF32FromDim(v2f pos, v2f scale)
{
	RectF32 out = {0};
	out.min.x = pos.x;
	out.min.y = pos.y;
	
	out.max.x = out.min.x + scale.x;
	out.max.y = out.min.y + scale.y;
	
	return out;
}

function v2f sizeFromRectF32(RectF32 rect)
{
	v2f out = {0};
	out.x = rect.max.x - rect.min.x;
	out.y = rect.max.y - rect.min.y;
	
	return out;
}

function v2f centerFromRect(RectF32 rect)
{
	v2f out = {0};
	
	out.x = (rect.max.x + rect.min.x) / 2.f;
	out.y = (rect.max.y + rect.min.y) / 2.f;
	
	return out;
}
