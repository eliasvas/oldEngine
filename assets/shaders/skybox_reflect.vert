#version 330 core

layout (location = 0) in vec3 vertex_pos; 
layout (location = 1) in vec3 n;
layout (location = 2) in vec2 tex_coord;
  
out vec2 f_tex_coord;
out vec3 f_normal;
out vec3 f_frag_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = vec4(vertex_pos,1.0);
	gl_Position = proj * view * model * gl_Position;

	f_tex_coord = tex_coord;
	f_normal = mat3(transpose(inverse(model))) * n;
	f_frag_pos = vec3(model * vec4(vertex_pos,1.0));
}
