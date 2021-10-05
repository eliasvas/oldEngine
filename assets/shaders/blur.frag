#version 330 core
layout (location = 0) out vec4 frag_color;
in vec2 f_tex_coord;

uniform sampler2D screen_texture;

uniform float flag;
uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);
uniform int blur_factor;


void main()
{
	int blur = blur_factor + 1; //because there is no easy way to make slider go [1-n] :))))))))))))))
	vec2 tex_size = 1.0 / textureSize(screen_texture, 0);
	vec3 blurred = vec3(0,0,0);
	for(int i = -blur; i < blur; ++i)
	{
		for (int j = -blur; j < blur; ++j)
		{
			vec2 offset = vec2(float(i), float(j)) * tex_size;
			blurred += texture(screen_texture, f_tex_coord + offset).xyz;
		}
	}
	///*
	frag_color = vec4(blurred / ((blur*2) * (blur*2)), 1);
}



