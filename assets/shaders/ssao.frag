#version 330 core
layout (location = 0) out vec4 frag_color;
in vec2 TexCoords;

uniform sampler2D depth_texture;
uniform sampler2D normal_texture;
uniform sampler2D position_texture;//we dont really need this
uniform sampler2D tex_noise;

uniform mat4 view;
uniform mat4 proj;
#define KERNEL_SIZE 64
#define RADIUS 1.0
uniform vec3 kernel[KERNEL_SIZE];

in vec3 view_ray;

float linearize_depth(float d)
{
	///*
	float A = proj[2].z;
    float B = proj[3].z;
    float zNear = - B / (1.0 - A);
    float zFar  =   B / (1.0 + A);
	return (2.0 * zNear) / (zFar + zNear - d * (zFar - zNear));	 
	//*/
	//return (0.5 * proj[3][2]) / (d + 0.5 * proj[2][2] - 0.5);
}

float rand(vec2 co){
    return fract(sin(dot(co, vec2(92.2898, 38.456))) * 43258.5453);
}

void main()
{
	float d = linearize_depth(texture(depth_texture, TexCoords).z);
	vec3 vr = normalize(view_ray);
	//vec3 positionVS = (vec4((vr * d),1) * view).xyz;
	vec3 positionVS = texture(position_texture, TexCoords).xyz;
	vec3 normal = texture(normal_texture, TexCoords).xyz;
	
	//vec3 random_vec = normalize(vec3(rand(TexCoords), rand(TexCoords), rand(TexCoords)));
	vec3 random_vec = normalize(texture(tex_noise, TexCoords * 1.0).xyz);
	//random_vec = normalize(vec3(0.2,0.5,0.3));
	vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal)); //should be sampled from noise texture
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);
	
	
	float occlusion = 0.0;
	for (int i = 0; i < KERNEL_SIZE; ++i)
	{
		vec3 sample_pos = TBN * kernel[i];
		sample_pos = positionVS + sample_pos * RADIUS;
		
		vec4 offset = vec4(sample_pos, 1.0);
		offset = proj * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;
		float sample_depth = texture(position_texture, offset.xy).z;
		
		occlusion += (sample_depth >= sample_pos.z + 0.1 ? 1.0 : 0.0);
	}
	occlusion = 1.0 - (occlusion/ KERNEL_SIZE);
	
	frag_color = vec4(occlusion);
	frag_color.a = 1.0;
	//frag_color = vec4(0.7);
}



