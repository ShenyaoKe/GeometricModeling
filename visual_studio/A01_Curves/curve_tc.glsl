#version 450
//layout(vertices = gl_MaxPatchVertices) out;
layout(vertices = 9) out;

uniform int segments;
uniform int NumStrips=1;

void main()
{
    // Pass along the vertex position unmodified
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    // Define the tessellation levels
    gl_TessLevelOuter[0] = float(NumStrips);
	gl_TessLevelOuter[1] = float(segments);
}