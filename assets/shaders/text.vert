#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 offset;
layout (location = 2) in vec2 dim;
layout (location = 3) in vec2 uv;

out vec2 f_tex_coords;
uniform mat4 MVP;

void main()
{
	f_tex_coords = position / 16;
	
	vec2 pos = position * dim;
	pos += vec2(offset);
	pos = (pos - 0.5) * 2;
    gl_Position = vec4(pos,0.0, 1.0); // see how we directly give a vec3 to vec4's constructor
    //gl_Position = MVP * gl_Position;
}
