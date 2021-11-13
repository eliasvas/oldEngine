#version 440
layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 bright_color;
  
in vec4 f_color;
in vec2 f_tex_coords;

void main()
{

	if (f_tex_coords.x * f_tex_coords.x + f_tex_coords.y * f_tex_coords.y > 1.f)discard;
	frag_color = f_color;
	
	
	float brightness = dot(frag_color.rgb, vec3(0.2126, 0.7152, 0.0722));
	if(brightness > 0.9)
        bright_color = vec4(frag_color.rgb, 1.0);
} 
