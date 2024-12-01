#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

layout(buffer_reference, std430) readonly buffer SceneData{ 
	layout(row_major) mat4 proj;
	layout(row_major) mat4 view;
	vec3 view_pos;
};

struct Vertex
{
	vec3 pos;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

layout( push_constant ) uniform constants
{	
	layout(row_major) mat4 render_matrix;
	SceneData scene;
	VertexBuffer vertexBuffer;
	uint tex_id;
} PushConstants;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out uint a_tex_id;
layout (location = 2) out vec2 a_uv;
layout (location = 3) out vec3 a_normal;
layout (location = 4) out vec3 a_frag_pos;
layout (location = 5) out vec3 a_view_pos;

void main() {
	
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	gl_Position = vec4(v.pos, 1.0f) * PushConstants.render_matrix * PushConstants.scene.view * PushConstants.scene.proj;
	
	fragColor = v.color.xyz;
	fragColor = v.normal;
	a_uv.x = v.uv_x;
	a_uv.y = v.uv_y;
	a_tex_id = PushConstants.tex_id;
	a_normal = mat3(PushConstants.render_matrix) * v.normal ;
	a_frag_pos = vec3(vec4(v.pos, 1.0f) * PushConstants.render_matrix);
	a_view_pos = PushConstants.scene.view_pos;
}