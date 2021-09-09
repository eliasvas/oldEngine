#version 330 core
out vec4 frag_color;
  
in vec2 f_tex_coords;
uniform sampler2D bmf_sampler;

float thickness = 0.5;
float softness = 0.1;
vec3 text_color = vec3(0.9,0.4,0.4);

float outline_thickness = 0.47;
float outline_softness = 0.1;
vec3 outline_color = vec3(0.1,0.1,0.1);
void main()
{
	frag_color = texture(bmf_sampler, f_tex_coords);
	//if (frag_color.a < 0.5)discard;
	
	
	
	
	float outline = smoothstep(1.0 - outline_thickness - outline_softness, 1.0 - outline_thickness + outline_softness, frag_color.a);
	
	float a = smoothstep(1.0 - thickness - softness, 1.0 - thickness + softness, frag_color.a);
	
	frag_color.a = a;
	frag_color.xyz = mix(outline_color, text_color, outline);
	
	
} 
