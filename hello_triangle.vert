#version 450
#extension GL_EXT_buffer_reference : require
layout (set = 0, binding = 0) readonly buffer scene_data_ssbo
{
    layout(row_major) mat4 proj;
    layout(row_major) mat4 view;
    layout(row_major) mat4 model[256];
    uint tex_id;
    uint pad;
};

layout(location = 0) out vec3 fragColor;
layout(location = 1) out uint a_tex_id;

vec2 positions[6] = vec2[](
                           vec2(-0.5, -0.5),
                           vec2(0.5, -0.5),
                           vec2(-0.5, 0.5),
                           
                           vec2(-0.5, 0.5),
                           vec2(0.5, 0.5),
                           vec2(0.5, -0.5)
                           );

vec3 colors[6] = vec3[](
                        vec3(1.0, 0.0, 0.0),
                        vec3(0.0, 1.0, 0.0),
                        vec3(0.0, 0.0, 1.0),
                        
                        vec3(1.0, 0.0, 0.0),
                        vec3(0.0, 1.0, 0.0),
                        vec3(0.0, 0.0, 1.0)
                        );

void main() {
    mat4 pv = view;
    gl_Position =  vec4(positions[gl_VertexIndex], 0, 1.0) * model[gl_InstanceIndex] * view * proj;
    //gl_Position =  vec4(positions[gl_VertexIndex], 0, 1.0) * view * proj;
    //gl_Position =  vec4(positions[gl_VertexIndex], 0, 1.0);
    fragColor = colors[gl_VertexIndex];
    a_tex_id = 0;
}