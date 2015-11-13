#version 430

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;

out vec3 normal;
out vec2 uv;

uniform mat4 view_matrix, proj_matrix;



void main()
{
	mat4 mvp = proj_matrix * view_matrix;
	// Transform each vertex into viewport space
	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	vec4 p2 = gl_in[2].gl_Position;
	vec4 p3 = gl_in[3].gl_Position;

	vec4 v1 = normalize(p1 - p0);
	vec4 v2 = normalize(p2 - p0);

	normal = normalize(cross(v1.xyz, v2.xyz));
	// Send the triangle along with the edge distances
	gl_Position = mvp * p0;
	uv = vec2(0, 0);
	EmitVertex();

	gl_Position = mvp * p1;
	uv = vec2(1, 0);
	EmitVertex();

	gl_Position = mvp * p3;
	uv = vec2(0, 1);
	EmitVertex();

	gl_Position = mvp * p2;
	uv = vec2(1, 1);
	EmitVertex();
	
	/*for (int i = 0; i < gl_in.length; i++)
	{
		gl_Position = mvp * gl_in[i].gl_Position;
		EmitVertex();
	}*/
	//EndPrimitive();
}