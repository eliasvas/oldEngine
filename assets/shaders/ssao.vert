#version 440
layout (location = 0) in vec2 vertex_pos;
layout (location = 1) in vec2 tex_coord;

out vec2 TexCoords;

out vec3 view_ray;
uniform mat4 view;
uniform mat4 proj;
uniform vec3 view_pos;

void main()
{
    gl_Position = vec4(vertex_pos.x, vertex_pos.y, 0.0, 1.0);
    TexCoords = tex_coord;
	mat4 inv_view_proj = inverse(proj)*inverse(view);
	vec4 positionWS = inverse(view) * inverse(proj) * vec4(vertex_pos, 1.0, 1.0);
	
	view_ray = positionWS.xyz - view_pos;
}