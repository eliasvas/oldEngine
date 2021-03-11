#version 330 core
out vec4 FragColor;
  
in vec2 f_tex_coords;
uniform sampler2D bmf_sampler;

void main()
{
	FragColor = texture(bmf_sampler, f_tex_coords);
	FragColor.xyz = vec3(0.9,0.3,0.3);
	//FragColor.w = 1.0;
	if (FragColor.x < 0.1)discard;
} 
