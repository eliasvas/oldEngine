#version 440
layout (location = 0) in vec3 vertex_pos;

uniform mat4 lsm;
uniform mat4 model;

void main()
{
    gl_Position = lsm * model * vec4(vertex_pos, 1.0);
}  