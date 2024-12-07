#version 450

layout(set = 0, binding = 0) uniform sampler2D tex[1000];

layout(location = 0) in vec3 fragColor;
layout(location = 1) flat in uint a_tex_id;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in vec3 a_normal;
layout (location = 4) in vec3 a_frag_pos;
layout (location = 5) in vec3 a_view_pos;
layout(location = 6) flat in uint a_normal_tex_id;
layout(location = 7) in vec3 a_tangent;
layout(location = 8) in vec3 a_bitangent;
layout(location = 9) in vec3 a_light_color;
layout(location = 10) in vec3 a_base_color;
layout(location = 11) in vec3 a_light_pos;

layout(location = 0) out vec4 out_color;

void main() {
	vec4 texture_color = texture(tex[a_tex_id], a_uv);
	vec4 normal_color = texture(tex[a_normal_tex_id], a_uv);
	
	float ambient_str = 0.1;
	vec3 ambient = ambient_str * a_light_color;
	
	vec3 norm = normalize(a_normal);
	vec3 light_dir = normalize(a_light_pos - a_frag_pos);
	float diff = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = diff * a_light_color;
	
	float specular_str = 0.5;
	vec3 view_dir = normalize(a_view_pos - a_frag_pos);
	vec3 reflect_dir = reflect(-light_dir, norm);
	
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 256);
	vec3 specular = specular_str * spec * a_light_color;  
	
	vec3 result = (ambient + diffuse + specular);
	
	out_color = texture_color * vec4(a_base_color, 1) * vec4(result, 1);
}