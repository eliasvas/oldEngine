#version 330 core
out vec4 FragColor;
  
in vec4 vertexColor;
in vec2 f_tex_coord;
uniform sampler2D sampler;

void main()
{
	FragColor = texture(sampler,f_tex_coord);
} 
