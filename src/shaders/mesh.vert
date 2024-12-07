#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "common.glsl"

layout( push_constant ) uniform constants
{	
	layout(row_major) mat4 render_matrix;
	SceneData scene;
	VertexBuffer v_buffer;
	uint tex_id;
	uint normal_id;
	uint pad1;
	uint pad2;
	vec3 base_color;
} pc;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out uint a_tex_id;
layout (location = 2) out vec2 a_uv;
layout (location = 3) out vec3 a_normal;
layout (location = 4) out vec3 a_frag_pos;
layout (location = 5) out vec3 a_view_pos;
layout(location = 6) flat out uint a_normal_tex_id;
layout(location = 7) out vec3 a_tangent;
layout(location = 8) out vec3 a_bitangent;
layout(location = 9) out vec3 a_light_color;
layout(location = 10) out vec3 a_base_color;
layout(location = 11) out vec3 a_light_pos;

void main() {
	
	Vertex v = pc.v_buffer.vertices[gl_VertexIndex];
	gl_Position = vec4(v.pos, 1.0f) * pc.render_matrix * pc.scene.view * pc.scene.proj;
	
	fragColor = v.color.xyz;
	fragColor = v.normal;
	a_uv.x = v.uv_x;
	a_uv.y = v.uv_y;
	a_tex_id = pc.tex_id;
	
	a_frag_pos = vec3(vec4(v.pos, 1.0f) * pc.render_matrix);
	a_normal = v.normal * mat3(transpose(inverse(pc.render_matrix)));
	
	a_view_pos = pc.scene.view_pos;
	a_normal_tex_id = pc.normal_id;
	a_tangent = v.tangent * mat3(pc.render_matrix);;
	
	a_bitangent = normalize(cross(a_normal, v.tangent));
	a_light_color = pc.scene.light_color;
	a_base_color = pc.base_color;
	a_light_pos = pc.scene.light_pos;
}