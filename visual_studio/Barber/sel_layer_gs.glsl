#version 430

layout(lines_adjacency) in;
layout(line_strip, max_vertices = 5) out;

void main()
{
	// Assign the same PID to both faces
	gl_PrimitiveID = gl_PrimitiveIDIn;
	
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	EmitVertex();

	gl_Position = gl_in[3].gl_Position;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
}