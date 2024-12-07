layout(buffer_reference, std430) readonly buffer SceneData{ 
	layout(row_major) mat4 proj;
	layout(row_major) mat4 view;
	vec3 view_pos;
	float pad;
	vec3 light_color;
	float pad2;
	vec3 light_pos;
	float pad3;
};

struct Vertex
{
	vec3 pos;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
	vec3 tangent;
	float pad;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

layout(set = 0, binding = 0) uniform sampler2D tex[1000];