#version 330 core

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 bright_color;
layout (location = 2) out vec4 position_vs;
in vec3 f_normal;
in vec3 f_frag_pos;
in vec3 f_frag_pos2;
void main()
{
	position_vs = vec4(f_frag_pos,1);
}
