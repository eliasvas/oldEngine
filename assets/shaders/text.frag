#version 330 core
out vec4 frag_color;
  
in vec2 f_tex_coord;
uniform sampler2D bmf_sampler;

void main()
{
	frag_color = texture(bmf_sampler, f_tex_coord);
	frag_color.xyz = vec3(0.9,0.3,0.3);
	//frag_color.w = 1.0;
	if (frag_color.x < 0.1)discard;
} 
