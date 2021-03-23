#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

out vec4 f_color;

uniform mat4 view;
uniform mat4 proj;
out vec3 f_frag_pos_ws;
out vec3 f_frag_pos;

void main(){
	f_frag_pos = vec3(proj * view * vec4(position, 1.0));
	f_frag_pos_ws = position;
	gl_Position = proj * view * vec4(position,1.0);
	f_color = color;

}

