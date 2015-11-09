#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

out vec3 normal;

uniform mat4 view_matrix, proj_matrix;



void main()
{
	mat4 mvp = proj_matrix * view_matrix;
	// Transform each vertex into viewport space
	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	vec4 p2 = gl_in[2].gl_Position;

	vec4 v1 = normalize(p1 - p0);
	vec4 v2 = normalize(p2 - p0);

	normal = normalize(cross(v1.xyz, v2.xyz));
	// Send the triangle along with the edge distances
	/*gl_Position = mvp * p0;
	EmitVertex();

	gl_Position = mvp * p1;
	EmitVertex();

	gl_Position = mvp * p2;
	EmitVertex();
	*/
	for (int i = 0; i < gl_in.length; i++)
	{
		gl_Position = mvp * gl_in[i].gl_Position;
		EmitVertex();
	}
	//EndPrimitive();
}