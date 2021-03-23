#version 430 core

in vec4 f_color;
struct NodeTypeLL
{

	float alpha;
	float blue;
	float green;
	float red;
	float depth;
	uint next;
};
out vec4 color;
layout(binding = 0, r32ui)		
coherent uniform uimage2D  in_image_head;


layout(binding = 1, std430)		
coherent buffer  LinkedLists   
{ 
NodeTypeLL nodes[]; 
};

layout(binding = 2, offset = 0)
uniform atomic_uint   in_next_address;



in vec3 f_frag_pos;
in vec3 f_frag_pos_ws;
uniform mat4 proj;
uniform mat4 view;
uniform int deep_render;
uniform vec3 view_front;
void main(){
	color = f_color;
	
	ivec2 coords = ivec2(gl_FragCoord.xy);

/*
//ENABLE FOR A-BUFFER WRITES
	// get next available location in global buffer
	uint index = atomicCounterIncrement(in_next_address) + 1U;
	if(index < nodes.length())
	{
		nodes[index].red = color.r;
		nodes[index].green = color.g;
		nodes[index].blue = color.b;
		nodes[index].alpha = color.a;
		if (deep_render > 0)
		{
			float A = proj[2].z;
			float B = proj[3].z;
			float zNear = (B + 1.0) / A;
			float zFar  =  (B - 1.0) / A;
			float t = (gl_FragCoord.z + 1.0) / 2.0;
			nodes[index].depth = zNear + t * (zFar - zNear);
		}
		else{
			nodes[index].depth = gl_FragCoord.z;
		}
		nodes[index].next  = imageAtomicExchange(in_image_head, ivec2(gl_FragCoord.xy), index);
	}
	discard;
*/
}