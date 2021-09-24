#version 330 core
out vec4 frag_color;
  
in vec4 vertex_color;
in vec2 f_tex_coord;
uniform sampler2D sampler;

void main()
{
	frag_color = texture(sampler,f_tex_coord);
} 
