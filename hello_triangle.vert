#version 450
#extension GL_EXT_buffer_reference : require
layout (set = 0, binding = 0) readonly buffer scene_data_ssbo
{
    layout(row_major) mat4 proj;
    layout(row_major) mat4 view;
    layout(row_major) mat4 model;
};

layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
                           vec2(0.0, 1/sqrt(3.0)),
                           vec2(-0.5, 1 / -(2.0 * sqrt(3.0))),
                           vec2(0.5, 1 / -(2.0 * sqrt(3.0)))
                           );

vec3 colors[3] = vec3[](
                        vec3(1.0, 0.0, 0.0),
                        vec3(0.0, 1.0, 0.0),
                        vec3(0.0, 0.0, 1.0)
                        );

void main() {
    mat4 pv = view;
    gl_Position =  vec4(positions[gl_VertexIndex], 0, 1.0) * model * view * proj;
    //gl_Position =  vec4(positions[gl_VertexIndex], 0, 1.0) * view * proj;
    //gl_Position =  vec4(positions[gl_VertexIndex], 0, 1.0);
    fragColor = colors[gl_VertexIndex];
}