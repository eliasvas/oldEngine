#version 330 core
out vec4 FragColor;

in vec2 f_tex_coord;
in vec4 f_vertex_color;

uniform sampler2D sampler;

void main()
{
	FragColor = texture(sampler,f_tex_coord);
	//FragColor = FragColor * 0.9 + f_vertex_color * 0.1;
	FragColor.a = 1.0;
}
