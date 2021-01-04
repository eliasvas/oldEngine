#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 uniform_view_matrix;
uniform mat4 uniform_projection_matrix;

void main()
{
	mat4 view = mat4(mat3(uniform_view_matrix));
    TexCoords = aPos;
    gl_Position = uniform_projection_matrix * view * vec4(aPos, 1.0);
}  