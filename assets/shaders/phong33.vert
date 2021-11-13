#version 440

layout (location = 0) in vec3 vertex_pos; 
layout (location = 1) in vec3 n;
layout (location = 2) in vec2 tex_coord;
  
out vec2 f_tex_coord;
out vec3 f_normal;
out vec3 f_frag_pos;
out vec4 f_frag_pos_ls;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 light_space_matrix;
out VS_OUT {
    vec3 normal;
} vs_out;
void main()
{
    gl_Position = vec4(vertex_pos,1.0);
	gl_Position = view * model * gl_Position;

	f_tex_coord = tex_coord;

	f_normal = mat3(transpose(inverse(model))) * n;
	mat3 normal_matrix = mat3(transpose(inverse(view * model)));
	f_frag_pos = vec3(model*vec4(vertex_pos,1.0));
	f_frag_pos_ls = light_space_matrix * vec4(f_frag_pos, 1.0);
	
	vs_out.normal = normalize(vec3(vec4(normal_matrix * n, 0.0)));
}
