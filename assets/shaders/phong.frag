#version 330 core
out vec4 FragColor;

in vec2 f_tex_coord;
in vec3 f_frag_pos;
in vec3 f_normal;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    //vec3 ambient;
    //vec3 diffuse;
    //vec3 specular;
    float shininess;
}; 
  
struct DirLight
{
   vec3 direction;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
};
struct PointLight
{
   vec3 position;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
};
#define MAX_POINT_LIGHTS 32
uniform vec3 view_pos;
uniform Material material;
uniform PointLight point_lights[MAX_POINT_LIGHTS];
uniform DirLight dirlight;
uniform int point_light_count;


void main()
{
	vec3 ambient = dirlight.ambient * vec3(texture(material.diffuse,f_tex_coord));
	
	vec3 n = normalize(f_normal);
	vec3 light_dir = normalize(-dirlight.direction);
	
	float diff = max(dot(n, light_dir), 0.0);
	vec3 diffuse = dirlight.diffuse * diff * vec3(texture(material.diffuse,f_tex_coord));
	
	vec3 view_dir = normalize(view_pos - f_frag_pos);
	vec3 reflect_dir = reflect(-light_dir, n);
	
	float spec = pow(max(dot(view_dir, reflect_dir),0.0),4);
	vec3 specular = dirlight.specular * spec * vec3(texture(material.specular,f_tex_coord));
	
	vec3 color = (ambient + diffuse + specular);

	//maybe have them per-light but I dont see the reason tbh..
	float constant = 1.f;
	float linear = 0.09;
	float quadratic = 0.032;

	for(int i = 0; i < point_light_count;++i)
	{
		ambient = point_lights[i].ambient * vec3(texture(material.diffuse,f_tex_coord));	
			
		n = normalize(f_normal);
		light_dir = normalize(point_lights[i].position - f_frag_pos);
		
		diff = max(dot(n, light_dir), 0.0);
		diffuse = point_lights[i].diffuse * diff * vec3(texture(material.diffuse,f_tex_coord));
		
		view_dir = normalize(view_pos - f_frag_pos);
		reflect_dir = reflect(-light_dir, n);
		
		spec = pow(max(dot(view_dir, reflect_dir),0.0),256);
		specular = point_lights[i].specular * spec * vec3(texture(material.specular,f_tex_coord));
		
		float distance = length(point_lights[i].position - f_frag_pos);
		float attenuation = 1.0/(constant + linear * distance + quadratic*(distance*distance));
		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
		color += (ambient + diffuse + specular);
	}
	FragColor = vec4(color,1.0);
}
