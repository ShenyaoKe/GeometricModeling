#version 430

layout(lines_adjacency) in;
layout(line_strip, max_vertices = 5) out;

uniform mat4 view_matrix, proj_matrix;

void main()
{
	mat4 mvp = proj_matrix * view_matrix;
	// Transform each vertex into viewport space
	vec4 p0 = mvp * gl_in[0].gl_Position;
	vec4 p1 = mvp * gl_in[1].gl_Position;
	vec4 p2 = mvp * gl_in[2].gl_Position;
	vec4 p3 = mvp * gl_in[3].gl_Position;
		
	gl_Position = p0;
	EmitVertex();

	gl_Position = p1;
	EmitVertex();

	gl_Position = p2;
	EmitVertex();

	gl_Position = p3;
	EmitVertex();

	gl_Position = p0;
	EmitVertex();
}