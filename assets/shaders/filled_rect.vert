#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 offset;
layout (location = 2) in vec4 color;

out vec4 f_color;

uniform mat4 MVP;

void main()
{
	vec2 pos = position*2 - 1.0;
	//pos /= 2.0;
    gl_Position = vec4(pos + offset,0.0, 1.0); // see how we directly give a vec3 to vec4's constructor
    //gl_Position = MVP * gl_Position;
	f_color = color;
}
