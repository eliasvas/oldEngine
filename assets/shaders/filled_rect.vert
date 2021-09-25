#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 offset;
layout (location = 2) in vec2 dim;
layout (location = 3) in vec4 color; //we also use color to store texture coordinates start/end if texture unit != -1
layout (location = 4) in int texture_unit;

out vec4 f_color;
out flat int f_tex_unit;

uniform mat4 MVP;

void main()
{
	vec2 pos = position * dim;
	pos += vec2(offset);
	pos = (pos - 0.5) * 2;
    gl_Position = vec4(pos,0.0, 1.0); // see how we directly give a vec3 to vec4's constructor
    //gl_Position = MVP * gl_Position;
	
	f_color = color;
	f_tex_unit = texture_unit;
}
