#version 330 core
out vec4 FragColor;
  
in vec2 f_tex_coords;
uniform sampler2D bmf_sampler;

void main()
{
	FragColor = texture(bmf_sampler, f_tex_coords);
	FragColor.y/=2.f;
	FragColor.z/=2.f;
	//FragColor.w = 1.0;
	if (FragColor.x < 0.1)discard;
} 
