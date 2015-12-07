#version 440
layout(vertices = gl_MaxPatchVertices) out;

uniform int segments = 200;
uniform int NumStrips = 30;
patch out int vertCount;
void main()
{
	vertCount = gl_PatchVerticesIn;
	// Pass along the vertex position unmodified
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	// Define the tessellation levels
	gl_TessLevelOuter[0] = float(NumStrips);
	gl_TessLevelOuter[1] = float(segments);
}