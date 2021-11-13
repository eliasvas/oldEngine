#version 440
layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 blurred_tex; //just for debugging
in vec2 f_tex_coord;

uniform sampler2D screen_texture;
uniform sampler2D bright_texture;
uniform sampler2D depth_texture;
uniform sampler2D ssao_texture;

uniform float flag;
uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);
uniform mat4 proj;

uniform float gamma;
uniform float exposure;
uniform int blur_factor;
float linearize_depth(float d)
{
	///*
	float A = proj[2].z;
    float B = proj[3].z;
    float zNear = - B / (1.0 - A);
    float zFar  =   B / (1.0 + A);
	return (2.0 * zNear) / (zFar + zNear - d * (zFar - zNear));	 
	//*/
	//return (0.5 * proj[3][2]) / (d + 0.5 * proj[2][2] - 0.5);
}
//return zNear * zFar / (zFar + d * (zNear - zFar));

void main()
{
	float g = gamma; //2.2
	float e = exposure; //1.0
	float tex_size = 1.0 / textureSize(bright_texture, 0).x;
	frag_color = texture(screen_texture, f_tex_coord);
	vec3 blurred = vec3(0);
	int blur = blur_factor + 1; //because there is no easy way to make slider go [1-n] :))))))))))))))
	for(int i = -blur; i < blur; ++i)
	{
		for (int j = -blur; j < blur; ++j)
		{
			vec2 offset = vec2(float(i), float(j)) * tex_size;
			blurred += texture(bright_texture, f_tex_coord + offset).xyz;
		}
	}
	blurred = blurred / ((blur*2) * (blur*2));
	///*
	frag_color.rgb += blurred;
	
	frag_color = vec4(pow(frag_color.xyz, vec3(g)), 1.0);

	
	
	frag_color.rgb = vec3(1.0) - exp(-frag_color.rgb * e);
	frag_color = vec4(pow(frag_color.xyz, vec3(1.0/gamma)), 1.0);
	//*/
	
	
	
    
	//frag_color = vec4(KernelColor, 1);
	gl_FragDepth = linearize_depth(max(0.05,texture(depth_texture, f_tex_coord).x));
	
	//frag_color = texture(ssao_texture, f_tex_coord);
}



