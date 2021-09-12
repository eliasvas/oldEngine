#version 330 core

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 bright_color;
layout (location = 2) out vec4 normal;
in vec3 f_normal;

void main()
{
	//gl_FragDepth = linearize_depth(gl_FragCoord.z);
	normal = normalize(vec4(f_normal,1));
}
