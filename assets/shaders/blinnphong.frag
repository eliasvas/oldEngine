#version 330 core
#define MAX_POINT_LIGHTS 512
#define CASCADES 3

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 bright_color;
in vec2 f_tex_coord;
in vec3 f_frag_pos;
in vec3 f_normal;
in vec4 f_frag_pos_ls[CASCADES];
in float clip_space_z;
in mat3 f_TBN;
struct Material {
    sampler2D diffuse_map;
    sampler2D specular_map;
	sampler2D bump_map;
    int has_specular_map;
	int has_diffuse_map;
	int has_bump_map;
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
struct VisibleIndex
{
	int index;
};

uniform mat4 proj;
uniform vec3 view_pos;
uniform Material material;
uniform DirLight dirlight;
uniform int point_light_count;
uniform sampler2D shadow_map[CASCADES];
uniform float cascade_ends_clip_space[CASCADES];
uniform int number_of_tiles_x;
uniform sampler2D ssao_texture;

layout(binding = 1, std430) buffer  light_buffer
{ 
	PointLight light_data[]; 
};
layout(binding = 2, std430) buffer  visible_index_buffer
{ 
	VisibleIndex light_index[]; 
};
float shadow_calc(int cascade_index)
{
	float bias = 0.01;
	//cascade_index = 0;
	//bias = max(0.05 * (1.0 - dot(f_normal, dirlight.direction)), 0.005);  
	// perform perspective divide (if ortho everything stays the same!)
    vec3 proj_coords = f_frag_pos_ls[cascade_index].xyz / f_frag_pos_ls[cascade_index].w;
	/*
	if (abs(proj_coords.z) >= 0.99)
	{
		return 1;
	}
	*/
    // transform to [0,1] range
    proj_coords = proj_coords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closest_depth = texture(shadow_map[cascade_index], proj_coords.xy).r; 
    // get depth of current fragment from light's perspective
    float current_depth = proj_coords.z;
    // check whether current frag pos is in shadow
    //float shadow = current_depth - bias > closest_depth  ? 1.0 : 0.0;
	
	
	///*
	float shadow = 0.0;
	vec2 texel_size = 1.0 / textureSize(shadow_map[cascade_index], 0);
	for(int x = -1; x <= 1; ++x)
	{
    for(int y = -1; y <= 1; ++y)
		{
			float pcf_depth = texture(shadow_map[cascade_index], proj_coords.xy + vec2(x, y) * texel_size).r; 
			shadow += current_depth - bias > pcf_depth ? 0.0 : 1.0;        
		}    
	}
	shadow /= 9.0; 
    return shadow;
}


void main()
{
	vec3 diffuse_color = material.diffuse*(1 - material.has_diffuse_map) + material.has_diffuse_map * vec3(texture(material.diffuse_map,f_tex_coord));
	vec3 specular_color = material.diffuse*(1 - material.has_specular_map) +material.has_specular_map * vec3(texture(material.specular_map,f_tex_coord));
	
	vec3 normal_vector = f_normal;
	if (material.has_bump_map)
	{
		normal_vector = texture(material.bump_map, f_tex_coord).xyz;
		//normal_vector  = normalize(normal_vector * 2.0 - 1.0);
		normal_vector = normalize(f_TBN * normal_vector);
	}
	
	vec3 ambient = dirlight.ambient * diffuse_color;
	
	vec3 N = normalize(normal_vector);
	vec3 L = normalize(-dirlight.direction);
	
	float diff = abs(dot(N, L));
	vec3 diffuse = dirlight.diffuse * diff * diffuse_color;
	
	vec3 V = normalize(view_pos - f_frag_pos);
	vec3 H = normalize(L + V);
	//vec3 R = reflect(-L, N);
	
	float spec = pow(max(dot(N, H),0.0),4);
	//float spec = pow(max(dot(V,R),0.0),4);
	vec3 specular = dirlight.specular * spec * specular_color;
	
	int cascade_index;
	for (int i = 0; i < CASCADES; ++i)
	{
		if (clip_space_z < cascade_ends_clip_space[i])
		{
			cascade_index = i; 
			diffuse[i] += 0.1f;
			break;
		}
	}
	
	float shadow = shadow_calc(cascade_index);
	
	vec3 color = (shadow*(specular + diffuse) + ambient);

	//maybe have them per-light but I dont see the reason tbh..
	float constant = 1.f;
	float linear = 0.09;
	float quadratic = 0.032;
	
	ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileID = location / ivec2(16, 16);
	uint index = tileID.y * number_of_tiles_x + tileID.x;
	uint offset = index * 1024;
	
	for(int i = 0; i < 1024 && light_index[offset + i].index != -1;++i)
	{
		//PointLight current_light = point_lights[i];
		uint light_idx = light_index[offset + i].index;
		PointLight current_light = light_data[light_idx];
		ambient = current_light.ambient * diffuse_color;	
		diffuse_color = current_light.diffuse;
		specular_color = current_light.specular;
		
		N = normalize(normal_vector);
		L = normalize(current_light.position - f_frag_pos);
		//R = reflect(-L, N);
		
		diff = max(dot(N,-L),0.0);
		diffuse = current_light.diffuse * diff * diffuse_color;
		
		V = normalize(view_pos - f_frag_pos);
		H = normalize(L + V);
		
		spec = pow(max(dot(N, H),0.0), 128);
		//spec = pow(max(dot(V,R),0.0),256);
		specular = current_light.specular * spec * specular_color;
		
		float distance = abs(length(current_light.position - f_frag_pos));
		float attenuation = 1.0/(constant + linear * distance + quadratic*(distance*distance));
		attenuation = 1.0/(distance * distance * distance * 32);
		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
		color += ((specular + diffuse) + ambient);
	}
	
	//gl_FragDepth = linearize_depth(gl_FragCoord.z);
	frag_color = vec4(color,1.0);
	
	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        bright_color = vec4(color.rgb,1.0);
    else
        bright_color = vec4(0.0, 0.0, 0.0, 1.0);
		
		
	//frag_color = texture(ssao_texture, f_tex_coord);
}