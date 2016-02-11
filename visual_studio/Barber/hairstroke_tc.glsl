#version 440
layout(vertices = gl_MaxPatchVertices) out;

uniform int segments = 200;
uniform int nStrips = 8;// Number of Strips
patch out int vertCount;
patch out int gridSample;
void main()
{
	vertCount = gl_PatchVerticesIn;
	gridSample = nStrips;
	// Pass along the vertex position unmodified
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	// Define the tessellation levels
	gl_TessLevelOuter[0] = float(nStrips * nStrips);
	gl_TessLevelOuter[1] = float(segments);
}