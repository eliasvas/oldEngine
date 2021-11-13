#version 440
out vec4 frag_color;

in vec3 tex_coord;

uniform samplerCube skybox;
void main()
{    
    frag_color = texture(skybox, -1.0 * tex_coord);
	gl_FragDepth = 0.99999; //i gotta linearize depth when i draw stuff so i dont need this (depth is packed close to 1.0)
}