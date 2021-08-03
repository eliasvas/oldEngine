#version 330 core

layout (location = 0) in vec3 vertex_pos;
layout (location = 1) in vec3 center;
layout (location = 2) in vec4 color;
//location = 4 vec2 dim pls!!
out vec4 f_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform vec3 cam_up;
uniform vec3 cam_right;
void main()
{
	//we orient the billboard so that it faces the camera!
	vec3 pos_ws = center + cam_right *vertex_pos.x + cam_up * vertex_pos.y;
	gl_Position = proj * view * vec4(pos_ws, 1);
	f_color = color;
}
