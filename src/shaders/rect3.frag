#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "common.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) flat in uint a_tex_id;
layout (location = 2) in vec2 a_uv;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 textureColor = texture(tex[a_tex_id], a_uv);
	
	outColor = textureColor;// * vec4(fragColor, 1);
}