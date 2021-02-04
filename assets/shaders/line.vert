#version 330 core

layout (location = 0) in vec3 start;
layout (location = 1) in vec3 end;
layout (location = 2) in vec4 color;

out vec4 f_color;

uniform mat4 view;

void main()
{
	vec3 vertices[] = vec3[](start, end);
	vec3 pos = vertices[gl_VertexID];
	pos *=2;
	pos -= 1.0;
	pos.y /=2.0;
	gl_Position = vec4(vec2(pos),0.0, 1.0); // see how we directly give a vec3 to vec4's constructor
    //gl_Position = MVP * gl_Position;
	f_color = color;
}
