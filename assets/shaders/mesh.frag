#version 330 core
out vec4 frag_color;

in vec2 f_tex_coord;
in vec4 f_vertex_color;

uniform sampler2D sampler;

void main()
{
	frag_color = texture(sampler,f_tex_coord);
	//frag_color = frag_color * 0.9 + f_vertex_color * 0.1;
	frag_color.a = 1.0;
}
