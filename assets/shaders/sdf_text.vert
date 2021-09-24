#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 offset;
layout (location = 2) in vec2 dim;
layout (location = 3) in vec2 uv;

out vec2 f_tex_coord;
uniform mat4 MVP;

void main()
{
	f_tex_coord = (1 - position) * uv + position * (uv + vec2(1.0/16.0));
	
	vec2 pos = position * dim;
	pos += vec2(offset);
	pos = (pos - 0.5) * 2;
    gl_Position = vec4(pos,0.0, 1.0);
    //gl_Position = MVP * gl_Position;
}
