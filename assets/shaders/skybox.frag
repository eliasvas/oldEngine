#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
void main()
{    
    FragColor = texture(skybox, -1.0 * TexCoords);
	gl_FragDepth = 0.99999; //i gotta linearize depth when i draw stuff so i dont need this (depth is packed close to 1.0)
}