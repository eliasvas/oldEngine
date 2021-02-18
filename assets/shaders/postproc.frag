#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

uniform float flag;
float offset = 1.0 / 300.0; //tweak this

uniform mat4 proj;
float linearize_depth(float d)
{
	float A = proj[2].z;
    float B = proj[3].z;
    float zNear = - B / (1.0 - A);
    float zFar  =   B / (1.0 + A);
	return (2.0 * zNear) / (zFar + zNear - d * (zFar - zNear));	 
}
//return zNear * zFar / (zFar + d * (zNear - zFar));


void main()
{
    //FragColor = texture(screenTexture, TexCoords);
	//FragColor = FragColor * (1 - flag) + vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0)* (flag);
	//enable this ^^^^^^ to get negative color
	
	FragColor = texture(screenTexture, TexCoords);
	vec3 KernelColor;
	vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );
	float kernel[9] = float[](
    1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
    2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
    1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0  
	);
	vec3 SampleTex[9];
	for (int i = 0; i < 9; ++i)
		SampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
	
	for (int  i = 0; i < 9; ++i)
		KernelColor += SampleTex[i] * kernel[i];
	
	FragColor = vec4(vec3(texture(screenTexture, TexCoords)), 1.0) * (1 - flag) + vec4(KernelColor,1.0) * (flag);
	//FragColor = vec4(KernelColor,1.0);
	
	float exposure = 2.0;
	float gamma = 2.0;
    //FragColor.rgb = pow(FragColor.rgb, vec3(gamma));
	
	// reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-FragColor.rgb * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(gamma));
	
    FragColor = vec4(mapped, 1.0);
	gl_FragDepth = linearize_depth(max(0.05,texture(depthTexture, TexCoords).x));
}