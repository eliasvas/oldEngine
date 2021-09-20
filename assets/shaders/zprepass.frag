#version 330 core

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 bright_color;
layout (location = 2) out vec4 normal;
in vec3 f_normal;
in vec3 f_frag_pos;
in vec3 f_frag_pos2;
void main()
{
	//gl_FragDepth = linearize_depth(gl_FragCoord.z);
	//gl_FragDepth = gl_FragCoord.z;
	//frag_color = vec4(f_frag_pos,1);
	normal = normalize(vec4(f_normal,1));
	//normal = vec4(0,1,0,1);
	bright_color = vec4(f_frag_pos,1);
}
