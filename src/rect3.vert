#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

layout(buffer_reference, std430) readonly buffer SceneData{ 
	layout(row_major) mat4 proj;
	layout(row_major) mat4 view;
};

layout(buffer_reference, std430) readonly buffer InstanceData{ 
	layout(row_major) mat4 model[4];
	uint tex_id[4];
};

layout( push_constant ) uniform constants
{
	SceneData scene;
	InstanceData instance;
} PushConstants;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out uint a_tex_id;
layout (location = 2) out vec2 a_uv;

vec2 positions[6] = vec2[](
                           vec2(-0.5, -0.5),
                           vec2(0.5, -0.5),
                           vec2(-0.5, 0.5),
                           
                           vec2(-0.5, 0.5),
                           vec2(0.5, -0.5),
                           vec2(0.5, 0.5)
                           );

vec2 uvs[6] = vec2[](
                     vec2(0, 0),
                     vec2(1, 0),
                     vec2(0, 1),
                     
                     vec2(0, 1),
                     vec2(1, 0),
                     vec2(1, 1)
                     );

vec3 colors[6] = vec3[](
                        vec3(1.0, 0.0, 0.0),
                        vec3(0.0, 1.0, 0.0),
                        vec3(0.0, 0.0, 1.0),
                        
                        vec3(1.0, 0.0, 0.0),
                        vec3(0.0, 0.0, 1.0),
                        vec3(0.0, 1.0, 0.0)
                        );

void main() {
	gl_Position =  vec4(positions[gl_VertexIndex], 0, 1.0) * PushConstants.instance.model[gl_InstanceIndex] * PushConstants.scene.view * PushConstants.scene.proj;
	//gl_Position =  vec4(positions[gl_VertexIndex], 0, 1.0) * view * proj;
	//gl_Position =  vec4(positions[gl_VertexIndex], 0, 1.0);
	fragColor = colors[gl_VertexIndex];
	a_tex_id = PushConstants.instance.tex_id[gl_InstanceIndex];
	a_uv = uvs[gl_VertexIndex];
}