#version 330 core

vec2 light_bias = vec2(0.5,0.5); //balance between ambient and diffuse..

in vec2 f_tex_coord;
in vec3 f_normal;
in vec4 color_vert;

out vec4 color;

uniform sampler2D diffuse_map;
uniform vec3 lightdir;

void main(){
	vec4 diffuse_color = texture(diffuse_map, f_tex_coord);
	vec3 unit_normal = normalize(f_normal);
	float diff_light = max(dot(-lightdir, unit_normal), 0.0) * light_bias.x + light_bias.y;
	color = diffuse_color * diff_light;
	color.a = 1.0;
}