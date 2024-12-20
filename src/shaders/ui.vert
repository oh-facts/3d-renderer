#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "common.glsl"

struct Rect
{
	vec2 tl;
	vec2 br;
};

struct Vertex2
{
	vec2 pos;
	vec2 uv;
	vec4 fade;
};

struct R_Rect2
{
	Rect src;
	Rect dst;
	vec4 border_color;
	vec4 fade[Corner_COUNT];
	uint tex_id;
	uint pad[3];
	float border_thickness;
	float radius;
	float pad2[2];
};

layout(buffer_reference, std430) readonly buffer InstanceData{ 
	layout(row_major) R_Rect2 rects[];
};

layout( push_constant ) uniform constants
{
	SceneData scene_data;
	InstanceData instance;
} PC;

layout(location = 0) out vec4 a_border_color;
layout(location = 1) out vec4 a_fade;
layout(location = 2) out vec2 a_uv;
layout(location = 3) flat out uint a_tex_id;
layout(location = 4) flat out float a_border_thickness;
layout(location = 5) flat out vec2 a_half_size;
layout(location = 6) flat out float a_radius;
layout(location = 7) out vec2 a_norm_uv;

void main()
{
	R_Rect2 obj = PC.instance.rects[gl_InstanceIndex];
	
	vec2 base_uv[] = 
	{
		{0, 0},
		{1, 0},
		{0, 1},
		
		{0, 1},
		{1, 0},
		{1, 1},
	};
	
	a_norm_uv = base_uv[gl_VertexIndex];
	
	Vertex2 vertices[] = 
	{
		{{ obj.dst.tl.x, obj.dst.tl.y}, {obj.src.tl.x, obj.src.br.y}, obj.fade[Corner_00]},
		{{ obj.dst.br.x, obj.dst.tl.y}, {obj.src.br.x, obj.src.br.y}, obj.fade[Corner_10]},
		{{ obj.dst.tl.x, obj.dst.br.y}, {obj.src.tl.x, obj.src.tl.y}, obj.fade[Corner_01]},
		
		{{ obj.dst.tl.x, obj.dst.br.y}, {obj.src.tl.x, obj.src.tl.y}, obj.fade[Corner_01]},
		{{ obj.dst.br.x, obj.dst.tl.y}, {obj.src.br.x, obj.src.br.y}, obj.fade[Corner_10]},
		{{ obj.dst.br.x, obj.dst.br.y}, {obj.src.br.x, obj.src.tl.y}, obj.fade[Corner_11]},
	};
	
	a_half_size = vec2((obj.dst.br.x - obj.dst.tl.x) * 0.5, (obj.dst.br.y - obj.dst.tl.y) * 0.5);
	
	Vertex2 vertex = vertices[gl_VertexIndex];
	
	a_tex_id = obj.tex_id;
	a_border_color = obj.border_color;
	a_fade = vertex.fade;
	a_border_thickness = obj.border_thickness;
	a_radius = obj.radius;
	a_uv = vertex.uv;
	vec2 norm_pos = vertex.pos ;/// PC.scene_data.screen_size.xy * 2.0 - 1.0;
	gl_Position = vec4(norm_pos, 0, 1) * PC.scene_data.view * PC.scene_data.proj;
}