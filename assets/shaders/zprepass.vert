#version 330 core

layout (location = 0) in vec3 vertex_pos; 
layout (location = 1) in vec3 n;
layout (location = 2) in vec2 tex_coord;
  
out vec2 f_tex_coord;
out vec3 f_normal;
out vec3 f_frag_pos;
out vec4 f_frag_pos_ls;
out vec3 f_frag_pos2;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 light_space_matrix;

void main()
{
    gl_Position = vec4(vertex_pos,1.0);
	gl_Position = proj * view * model * gl_Position;
	f_frag_pos2 = gl_Position.xyz;
	f_tex_coord = tex_coord;

	f_normal = mat3(transpose(inverse(mat3(model)))) * n;
	//f_normal = (view * vec4(f_normal, 1)).xyz;
	f_frag_pos = vec3(view * model*vec4(vertex_pos,1.0)); //we output view space position
	f_frag_pos_ls = light_space_matrix * vec4(f_frag_pos, 1.0);
	//gl_Position.z = linearize_depth(gl_Position.z);
}
