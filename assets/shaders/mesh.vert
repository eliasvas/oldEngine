#version 330 core

layout (location = 0) in vec3 vertex_pos; 
layout (location = 1) in vec3 n;
layout (location = 2) in vec2 tex_coord;
  
out vec2 f_tex_coord;
out vec4 f_vertex_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = vec4(vertex_pos,1.0);
	gl_Position = proj * view * model * gl_Position;

    f_vertex_color = vec4(normalize(n), 1.0);
	f_tex_coord = tex_coord;
}
