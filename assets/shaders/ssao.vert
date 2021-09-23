#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

out vec3 view_ray;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 view_pos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
	mat4 inv_view_proj = inverse(proj)*inverse(view);
	vec4 positionWS = inverse(view) * inverse(proj) * vec4(aPos, 1.0, 1.0);
	
	view_ray = positionWS.xyz - view_pos;
}