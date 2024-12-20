#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "common.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec4 a_border_color;
layout(location = 1) in vec4 a_fade;
layout(location = 2) in vec2 a_uv;
layout(location = 3) flat in uint a_tex_id;
layout(location = 4) flat in float a_border_thickness;
layout(location = 5) flat in vec2 a_half_size;
layout(location = 6) flat in float a_radius;
layout(location = 7) in vec2 a_norm_uv;

float RectSDF(vec2 p, vec2 b, float r)
{
	vec2 d = abs(p) - b + vec2(r);
	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;   
}

void main() 
{
	vec4 tex_col = texture(sampler2Ds[a_tex_id], a_uv);
	
	vec2 pos = a_half_size * 2 * a_norm_uv;
	
	float fDist = RectSDF(pos - a_half_size, a_half_size - a_border_thickness/2.0, a_radius);
	float fBlendAmount = smoothstep(-1.0, 0.0, abs(fDist) - a_border_thickness / 2.0);
  
	vec4 v4FromColor = a_border_color;
	vec4 v4ToColor = (fDist < 0.0) ? a_fade * tex_col : vec4(0);
	out_color = mix(v4FromColor, v4ToColor, fBlendAmount);
}