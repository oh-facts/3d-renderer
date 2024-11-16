typedef union V2F V2F;
union V2F
{
	f32 e[2];
	struct
	{
		f32 x;
		f32 y;
	};
};

typedef union V2S V2S;
union V2S
{
	s32 e[2];
	struct
	{
		s32 x;
		s32 y;
	};
};

function b32 v2s_equals(V2S a, V2S b)
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

typedef union V3F V3F;
union V3F
{
	f32 e[3];
	struct
	{
		f32 x;
		f32 y;
        f32 z;
	};
};

function V3F v3f(f32 x, f32 y, f32 z)
{
    V3F out = {.x = x, .y = y, .z = z};
    return out;
}

typedef struct RectF32 RectF32;
struct RectF32
{
	V2F min;
	V2F max;
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

function RectF32 rectF32FromDim(V2F pos, V2F scale)
{
	RectF32 out = {0};
	out.min.x = pos.x;
	out.min.y = pos.y;
	
	out.max.x = out.min.x + scale.x;
	out.max.y = out.min.y + scale.y;
	
	return out;
}

function V2F sizeFromRectF32(RectF32 rect)
{
	V2F out = {0};
	out.x = rect.max.x - rect.min.x;
	out.y = rect.max.y - rect.min.y;
	
	return out;
}

function V2F centerFromRect(RectF32 rect)
{
	V2F out = {0};
	
	out.x = (rect.max.x + rect.min.x) / 2.f;
	out.y = (rect.max.y + rect.min.y) / 2.f;
	
	return out;
}

typedef struct M4F M4F;
struct M4F
{
    f32 v[4][4];
};

function M4F m4f(f32 v)
{
    M4F out = {
        .v[0][0] = v,
        .v[1][1] = v,
        .v[2][2] = v,
        .v[3][3] = v,
    };
    return out;
}

function M4F m4f_scale(f32 v)
{
    M4F out = {
        .v[0][0] = v,
        .v[1][1] = v,
        .v[2][2] = v,
        .v[3][3] = 1,
    };
    return out;
}

#undef near
#undef far

function f32 v3f_length(V3F v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

function V3F v3f_add(V3F a, V3F b)
{
    V3F out = {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z,
    };
    
    return out;
}

function V3F v3f_sub(V3F a, V3F b)
{
    V3F out = {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z,
    };
    
    return out;
}

function V3F v3f_normalize(V3F v)
{
    V3F out = v;
    
    f32 len = v3f_length(v);
    
    if(len != 0)
    {
        out.x /= len;
        out.y /= len;
        out.z /= len;
    }
    return out;
}

function f32 v3f_dot(V3F a, V3F b)
{
    f32 out = a.x * b.x + a.y * b.y + a.z * b.z;
    
    return out;
}

function V3F v3f_cross(V3F a, V3F b)
{
    V3F out = {
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x * b.y - a.y * b.x,
    };
    
    return out;
}

function M4F
m4f_lookAt(V3F eye, V3F center, V3F up)
{
    V3F f = v3f_normalize(v3f_sub(eye, center));
    V3F s = v3f_normalize(v3f_cross(f, up));
    V3F u = v3f_cross(s, f);
    
    M4F out = {
        .v[0][0] = s.x,
        .v[0][1] = u.x,
        .v[0][2] = -f.x,
        .v[0][3] = 0.0f,
        
        .v[1][0] = s.y,
        .v[1][1] = u.y,
        .v[1][2] = -f.y,
        .v[1][3] = 0.0f,
        
        .v[2][0] = s.z,
        .v[2][1] = u.z,
        .v[2][2] = -f.z,
        .v[2][3] = 0.0f,
        
        .v[3][0] = -v3f_dot(s, eye),
        .v[3][1] = -v3f_dot(u, eye),
        .v[3][2] = v3f_dot(f, eye),
        .v[3][3] = 1.0f,
    };
    
    return out;
}

function M4F
m4f_perspective(f32 fov, f32 aspect_ratio, f32 near_z, f32 far_z)
{
    M4F result = m4f(1.f);
    f32 tan_theta_over_2 = tan(fov / 2);
    result.v[0][0] = 1.f / tan_theta_over_2;
    result.v[1][1] = aspect_ratio / tan_theta_over_2;
    result.v[2][3] = 1.f;
    result.v[2][2] = -(near_z + far_z) / (near_z - far_z);
    result.v[3][2] = (2.f * near_z * far_z) / (near_z - far_z);
    result.v[3][3] = 0.f;
    return result;
}

function M4F m4f_translate(V3F v)
{
	return (M4F){
		{
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{v.x, v.y, v.z, 1},
		}
	};
}

function M4F m4f_rotate(V3F axis, f32 rad)
{
    axis = v3f_normalize(axis);
    f32 sin_theta = sin(rad);
    f32 cos_theta = cos(rad);
    f32 one_minus_cos = 1.f - cos_theta;
    
    M4F out = {
        .v[0][0] = (axis.x * axis.x * one_minus_cos) + cos_theta,
        .v[0][1] = (axis.x * axis.y * one_minus_cos) + (axis.z * sin_theta),
        .v[0][2] = (axis.x * axis.z * one_minus_cos) - (axis.y * sin_theta),
        
        .v[1][0] = (axis.y * axis.x * one_minus_cos) - (axis.z * sin_theta),
        .v[1][1] = (axis.y * axis.y * one_minus_cos) + cos_theta,
        .v[1][2] = (axis.y * axis.z * one_minus_cos) + (axis.x * sin_theta),
        
        .v[2][0] = (axis.z * axis.x * one_minus_cos) + (axis.y * sin_theta),
        .v[2][1] = (axis.z * axis.y * one_minus_cos) - (axis.x * sin_theta),
        .v[2][2] = (axis.z * axis.z * one_minus_cos) + cos_theta,
        
        .v[3][3] = 1,
    };
    
    return out;
}

function M4F m4f_mul(M4F a, M4F b)
{
    M4F out = {0};
	
    for(int j = 0; j < 4; j += 1)
    {
        for(int i = 0; i < 4; i += 1)
        {
            out.v[i][j] = (a.v[0][j]*b.v[i][0] +
                           a.v[1][j]*b.v[i][1] +
                           a.v[2][j]*b.v[i][2] +
                           a.v[3][j]*b.v[i][3]);
        }
    }
    
	return out;
}
