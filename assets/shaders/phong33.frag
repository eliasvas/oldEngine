#version 330 core
out vec4 frag_color;

in vec2 f_tex_coord;
in vec3 f_frag_pos;
in vec3 f_normal;
in vec4 f_frag_pos_ls;
in vec4 g_color;

struct Material {
    sampler2D diffuse_map;
    sampler2D specular_map;
    int has_specular_map;
	int has_diffuse_map;
	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

struct DirLight
{
   vec3 direction;

   vec3 ambient;
   vec3 diffuse;
   vec3 specular;
};
struct PointLight {    
    vec3 position;
    float constant;
    vec3 ambient;
    float linear;
    vec3 diffuse;
    float quadratic;
    vec3 specular;
    float shininess;
}; 
#define MAX_POINT_LIGHTS 1024
uniform mat4 proj;
uniform vec3 view_pos;
uniform Material material;
uniform PointLight point_lights[MAX_POINT_LIGHTS];
uniform DirLight dirlight;
uniform int point_light_count;
uniform sampler2D shadow_map;
float shadow_calc()
{
	float bias = 0.005;
	// perform perspective divide
    vec3 proj_coords = f_frag_pos_ls.xyz / f_frag_pos_ls.w;
    // transform to [0,1] range
    proj_coords = proj_coords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closest_depth = texture(shadow_map, proj_coords.xy).r; 
    // get depth of current fragment from light's perspective
    float current_depth = proj_coords.z;
    // check whether current frag pos is in shadow
    //float shadow = current_depth - bias > closest_depth  ? 1.0 : 0.0;
	
	float shadow = 0.0;
	vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
	for(int x = -1; x <= 1; ++x)
	{
    for(int y = -1; y <= 1; ++y)
		{
			float pcf_depth = texture(shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r; 
			shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

    return 1.0 - shadow;
}


void main()
{
	frag_color = texture(material.diffuse_map, f_tex_coord);
	return;
	vec3 ambient = dirlight.ambient * vec3(texture(material.diffuse_map,f_tex_coord));
	
	vec3 n = normalize(f_normal);
	vec3 light_dir = normalize(-dirlight.direction);
	
	float diff = max(dot(n, light_dir), 0.0);
	vec3 diffuse = dirlight.diffuse * diff * vec3(texture(material.diffuse_map,f_tex_coord));
	
	vec3 view_dir = normalize(view_pos - f_frag_pos);
	vec3 reflect_dir = reflect(-light_dir, n);
	
	float spec = pow(max(dot(view_dir, reflect_dir),0.0),4);
	vec3 specular = dirlight.specular * spec * vec3(texture(material.specular_map,f_tex_coord));
	
	float shadow = shadow_calc();
	
	vec3 color = (shadow*(specular + diffuse) + ambient)/2;

	//maybe have them per-light but I dont see the reason tbh..
	float constant = 1.f;
	float linear = 0.09;
	float quadratic = 0.032;
	
	
	for(int i = 0; i < point_light_count;++i)
	{
		ambient = point_lights[i].ambient * vec3(texture(material.diffuse_map,f_tex_coord));	
			
		n = normalize(f_normal);
		light_dir = normalize(point_lights[i].position - f_frag_pos);
		
		diff = max(dot(n,-light_dir),0.0);
		diffuse = point_lights[i].diffuse * diff * vec3(texture(material.diffuse_map,f_tex_coord));
		
		view_dir = normalize(view_pos - f_frag_pos);
		reflect_dir = reflect(-light_dir, n);
		
		spec = pow(max(dot(view_dir, reflect_dir),0.0),256);
		specular = point_lights[i].specular * spec * vec3(texture(material.specular_map,f_tex_coord));
		
		float distance = abs(length(point_lights[i].position - f_frag_pos));
		float attenuation = 1.0/(constant + linear * distance + quadratic*(distance*distance));
		attenuation = 1.0/(distance);
		ambient *= attenuation * 0.02;
		diffuse *= attenuation * 0.02;
		specular *= attenuation * 0.02;
		color += ((specular + diffuse) + ambient);
	}
	frag_color = vec4(color,texture(material.diffuse_map,f_tex_coord).a);
}
