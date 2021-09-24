#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 tex_coord;

out vec2 f_tex_coord;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    f_tex_coord = tex_coord;
}  