#version 330 core
out vec4 FragColor;

in vec2 f_tex_coord;
in vec3 f_frag_pos;
in vec3 f_normal;
uniform vec3 view_pos;
uniform samplerCube skybox;
void main()
{
	vec3 I = normalize(f_frag_pos - view_pos);
    vec3 R = reflect(I, normalize(f_normal));
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}