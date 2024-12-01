#version 450

layout(set = 0, binding = 0) uniform sampler2D tex[1000];

layout(location = 0) in vec3 fragColor;
layout(location = 1) flat in uint a_tex_id;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in vec3 a_normal;
layout (location = 4) in vec3 a_frag_pos;
layout (location = 5) in vec3 a_view_pos;

layout(location = 0) out vec4 outColor;

void main() {
	
	float ambient_str = 0.1f;
	vec3 ambient_color = vec3(1, 1, 1);
	vec3 ambient = ambient_str * ambient_color;
	vec3 light_color = vec3(1, 1, 1);
	
	vec3 light_pos = vec3(8.68, 1.4, -0.28);
	
	vec3 norm = normalize(a_normal);
	
	vec3 light_dir = normalize(light_pos - a_frag_pos);
	float diff = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = diff * light_color;
	
	float specular_str = 0.5;
	
	vec3 view_dir = normalize(a_view_pos - a_frag_pos);
	vec3 reflect_dir = reflect(-light_dir, norm);  
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
	vec3 specular = specular_str * spec * light_color;  
	
	vec4 lighting_color = vec4(ambient + diffuse + specular, 1);
	vec4 textureColor = texture(tex[a_tex_id], a_uv);
	
	vec3 debug_normals = norm * 0.5 + 0.5;
	
	outColor = lighting_color * textureColor;//vec4(debug_normals, 1);//lighting_color * textureColor;//vec4(1);//textureColor;// * vec4(fragColor, 1);
}