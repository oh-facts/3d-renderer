#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

layout(buffer_reference, std430) readonly buffer SceneData{ 
	layout(row_major) mat4 proj;
	layout(row_major) mat4 view;
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

layout(location = 0) out vec3 fragColor;
layout(location = 1) out uint a_tex_id;
layout (location = 2) out vec2 a_uv;

void main() {
	
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	gl_Position = vec4(v.pos, 1.0f) * PushConstants.render_matrix * PushConstants.scene.view * PushConstants.scene.proj;
	
	fragColor = v.color.xyz;
	//fragColor = v.normal;
	a_uv.x = v.uv_x;
	a_uv.y = v.uv_y;
	a_tex_id = PushConstants.tex_id;
}