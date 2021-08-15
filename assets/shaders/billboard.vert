#version 330 core

layout (location = 0) in vec3 vertex_pos;
layout (location = 1) in vec3 center;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 dim;
out vec4 f_color;
out vec2 f_tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 cam_up;
uniform vec3 cam_right;
void main()
{
	//we orient the billboard so that it faces the camera!
	vec3 pos_ws = center + cam_right *vertex_pos.x * dim.x + cam_up * vertex_pos.y * dim.y;
	gl_Position = proj * view * vec4(pos_ws, 1);
	f_color = color;
	vec2 tex_coords = vec2(vertex_pos.x*2.0, vertex_pos.y*2.0);
	f_tex_coords = tex_coords;
}
