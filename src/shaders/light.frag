#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "common.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) flat in uint a_tex_id;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in vec3 a_normal;
layout (location = 4) in vec3 a_frag_pos;
layout (location = 5) in vec3 a_view_pos;
layout(location = 6) flat in uint a_normal_tex_id;
layout(location = 7) in vec3 a_tangent;
layout(location = 8) in vec3 a_bitangent;
layout(location = 9) in vec3 a_base_color;

layout(location = 0) out vec4 out_color;

void main() {
	vec4 texture_color = texture(tex[a_tex_id], a_uv);
	vec4 normal_color = texture(tex[a_normal_tex_id], a_uv);
	
	out_color = vec4(a_base_color, 1);
}