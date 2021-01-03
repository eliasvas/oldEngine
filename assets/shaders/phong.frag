#version 330 core
out vec4 FragColor;

in vec2 f_tex_coord;
in vec3 f_frag_pos;
in vec3 f_normal;

uniform sampler2D sampler;
uniform sampler2D sampler2;
uniform vec3 light_pos;
uniform vec3 view_pos;

void main()
{
	vec3 light_color = vec3(0.6,0.6,0.5);
	
	float ambient_str = 0.1;
	vec3 ambient = ambient_str * light_color;
	
	vec3 n = normalize(f_normal);
	vec3 light_dir = normalize(light_pos - f_frag_pos);
	
	float diff = max(dot(n, light_dir), 0.0);
	vec3 diffuse = light_color * diff;
	
	float specular_str = 1.0;
	vec3 view_dir = normalize(view_pos - f_frag_pos);
	vec3 reflect_dir = reflect(-light_dir, n);
	
	float spec = pow(max(dot(view_dir, reflect_dir),0.0),4);
	vec3 specular = specular_str * spec * light_color;
	
	vec3 color = (ambient + diffuse ) * vec3(texture(sampler,f_tex_coord));
	color += (specular ) * vec3(texture(sampler2,f_tex_coord));
	
	FragColor = vec4(color,1.0);
}
