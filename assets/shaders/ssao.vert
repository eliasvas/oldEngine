#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

out vec3 view_ray;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
	mat4 inv_view_proj = inverse(view * proj);
	view_ray = (vec4(aPos, 1, 1) * inv_view_proj).xyz;
}