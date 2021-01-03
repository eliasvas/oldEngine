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
  

struct PointLight
{
   vec3 position;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
};
uniform vec3 view_pos;
uniform Material material;
uniform PointLight light;


void main()
{
	
	vec3 ambient = light.ambient * vec3(texture(material.diffuse,f_tex_coord));
	
	vec3 n = normalize(f_normal);
	vec3 light_dir = normalize(light.position - f_frag_pos);
	
	float diff = max(dot(n, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse,f_tex_coord));
	
	vec3 view_dir = normalize(view_pos - f_frag_pos);
	vec3 reflect_dir = reflect(-light_dir, n);
	
	float spec = pow(max(dot(view_dir, reflect_dir),0.0),256);
	vec3 specular = light.specular * spec * vec3(texture(material.specular,f_tex_coord));
	
	vec3 color = ambient + diffuse + specular;
	
	FragColor = vec4(color,1.0);
}
