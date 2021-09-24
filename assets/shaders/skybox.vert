#version 330 core
layout (location = 0) in vec3 vertex_pos;

out vec3 tex_coord;

uniform mat4 uniform_view_matrix;
uniform mat4 uniform_projection_matrix;

void main()
{
	mat4 view = mat4(mat3(uniform_view_matrix));
    tex_coord = vertex_pos;
    gl_Position = uniform_projection_matrix * view * vec4(vertex_pos, 1.0);
	gl_Position = gl_Position.xyww;
	
}  