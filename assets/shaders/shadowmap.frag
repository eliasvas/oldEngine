#version 330 core

void main()
{     
	//discard;
    gl_FragDepth = gl_FragCoord.z;
}  