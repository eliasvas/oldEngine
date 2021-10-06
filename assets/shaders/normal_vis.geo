

#version 330
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;
in VS_OUT {
    vec3 normal;
} gs_in[];

uniform mat4 proj;

void build_normals()
{
	vec4 pos = gl_in[0].gl_Position;
	gl_Position = pos + vec4(0.0,0.0,0.0,0.0);
	EmitVertex();
	gl_Position = pos + vec4(gs_in[0].normal,1);
	EmitVertex();
	EndPrimitive();
	pos = gl_in[1].gl_Position;
	gl_Position = pos + vec4(0.0,0.0,0.0,0.0);
	EmitVertex();
	gl_Position = pos + vec4(gs_in[1].normal,1);
	EmitVertex();
	EndPrimitive();
	pos = gl_in[0].gl_Position;
	gl_Position = pos + vec4(0.0,0.0,0.0,0.0);
	EmitVertex();
	
	gl_Position = pos + vec4(gs_in[2].normal,1);
	EmitVertex();
	EndPrimitive();
}
const float MAGNITUDE = 0.5; //how much further along the normal we will go

void generate_line(int index)
{
	gl_Position = proj * gl_in[index].gl_Position;
	EmitVertex();
	gl_Position = proj * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0)*MAGNITUDE);
	EmitVertex();
	EndPrimitive();
}
void main() {
    //build_normals();
	generate_line(0);
	generate_line(1);
	generate_line(2);
}