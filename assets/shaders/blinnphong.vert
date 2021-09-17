#version 330 core

layout (location = 0) in vec3 vertex_pos; 
layout (location = 1) in vec3 n;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in vec3 tangent;

#define CASCADES 3
out vec2 f_tex_coord;
out vec3 f_normal;
out vec3 f_frag_pos;
out vec4 f_frag_pos_ls[CASCADES];
out float clip_space_z;
out mat3 f_TBN;
out vec2 screen_space_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 light_space_matrix[CASCADES];

void main()
{
	gl_Position = proj * view * model * vec4(vertex_pos,1.0);
	screen_space_pos = (gl_Position / gl_Position.w).xy;
	screen_space_pos = screen_space_pos * 0.5 + 0.5;
	f_tex_coord = tex_coord;

	f_normal = mat3(transpose(inverse(model))) * n;
	f_frag_pos = vec3(model*vec4(vertex_pos,1.0));
	for (int i = 0; i < CASCADES; ++i)
	{
		f_frag_pos_ls[i] = light_space_matrix[i] * model*vec4(vertex_pos,1.0);
	}
	clip_space_z = gl_Position.z;/// gl_Position.w;
	
	vec3 T = normalize(vec3(model * vec4(tangent,0.0)));
	vec3 N = normalize(vec3(model * vec4(n,0.0)));
	vec3 B = normalize(cross(N,T));
	mat3 TBN = mat3(T,B,N);
	f_TBN = TBN;
}
