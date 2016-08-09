#version 440
#define ID gl_InvocationID
layout(vertices = gl_MaxPatchVertices) out;
uniform int segments;
uniform int NumStrips = 1;

void main()
{
	// Pass along the vertex position unmodified
	gl_out[ID].gl_Position = gl_in[ID].gl_Position;
	// Define the tessellation levels
	gl_TessLevelOuter[0] = float(NumStrips);
	gl_TessLevelOuter[1] = float(segments);
}