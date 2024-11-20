#version 450

layout(set = 0, binding = 1) uniform sampler2D textures[];

layout(location = 0) in vec3 fragColor;
layout(location = 1) flat in uint a_tex_id;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}