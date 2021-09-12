#version 330 core
layout (location = 0) out vec4 frag_color;
in vec2 TexCoords;

uniform sampler2D depth_texture;
uniform sampler2D normal_texture;

uniform mat4 view;
uniform mat4 proj;

void main()
{

	frag_color = texture(normal_texture, TexCoords);
}



