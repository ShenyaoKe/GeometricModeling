#version 440
#define ID gl_InvocationID
layout(vertices = 4) out;
uniform int segments;
uniform int NumStrips = 1;
patch out float dist[4];

void main()
{
	// Pass along the vertex position unmodified
	gl_out[ID].gl_Position = gl_in[ID].gl_Position;
	// Define the tessellation levels
	gl_TessLevelOuter[0] = float(NumStrips);
	gl_TessLevelOuter[1] = float(segments);
	if (ID > 0) dist[ID] = sqrt(length(gl_in[ID].gl_Position.xyz
	                             - gl_in[ID - 1].gl_Position.xyz));
	else        dist[ID] = 0;
}