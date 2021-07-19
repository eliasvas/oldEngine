#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 blurred_tex; //just for debugging
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D brightTexture;
uniform sampler2D depthTexture;

uniform float flag;
uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);
uniform mat4 proj;
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
    //FragColor = texture(screenTexture, TexCoords);
	//FragColor = FragColor * (1 - flag) + vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0)* (flag);
	//enable this ^^^^^^ to get negative color
	float offset = 1.0 / textureSize(brightTexture, 0).x;
	FragColor = texture(screenTexture, TexCoords);

//	/*
	vec3 blurred;
	for(int i = 1; i < 8; ++i)
    {
       blurred += texture(brightTexture, TexCoords + vec2(offset.x * i, 0.0)).rgb * weight[i];
       blurred += texture(brightTexture, TexCoords - vec2(offset.x * i, 0.0)).rgb * weight[i];
    }

	float exposure = 2.0;
	float gamma = 2.2;
	blurred_tex = vec4(blurred,1.0);
	//FragColor += vec4(blurred,1.0);
	// reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-FragColor.rgb * exposure);
    // gamma correction 
    FragColor = vec4(pow(mapped, vec3(gamma)), 1.0); //1.0 / gamma if SRGB??
//	*/
    
	//FragColor = vec4(KernelColor, 1);
	gl_FragDepth = linearize_depth(max(0.05,texture(depthTexture, TexCoords).x));
}



