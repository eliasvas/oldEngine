#version 330 core
out vec4 frag_color;
  
in vec4 f_color;
in vec2 f_tex_coord;
in flat int f_tex_unit;


uniform sampler2D sampler[16];

void main()
{
	frag_color = f_color * (1 - sign(f_tex_unit));
} 
