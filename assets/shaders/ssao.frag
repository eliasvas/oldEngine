#version 330 core
layout (location = 0) out vec4 frag_color;
in vec2 TexCoords;

uniform sampler2D depth_texture;
uniform sampler2D normal_texture;

uniform mat4 view;
uniform mat4 proj;

uniform vec3 kernel[64];

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

void main()
{
	float d = texture(depth_texture, TexCoords).x;
	
	
	frag_color = texture(normal_texture, TexCoords);
	frag_color.xyz +=kernel[33] / 100.0;
	frag_color = vec4(vec3(linearize_depth(d)), 1.0);
}



