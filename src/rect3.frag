#version 450

layout(set = 0, binding = 0) uniform sampler2D tex[1000];

layout(location = 0) in vec3 fragColor;
layout(location = 1) flat in uint a_tex_id;
layout (location = 2) in vec2 a_uv;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 textureColor = texture(tex[a_tex_id], a_uv);
	
	outColor = textureColor;// * vec4(fragColor, 1);
}