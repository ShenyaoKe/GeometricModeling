#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

out vec3 normal;
out vec2 uv;
noperspective out vec3 GEdgeDistance;

uniform mat4 view_matrix, proj_matrix;



void main()
{
	mat4 mvp = proj_matrix * view_matrix;
	// Transform each vertex into viewport space
	vec4 p0 = mvp * gl_in[0].gl_Position;
	vec4 p1 = mvp * gl_in[1].gl_Position;
	vec4 p2 = mvp * gl_in[2].gl_Position;

	vec4 v1 = normalize(gl_in[1].gl_Position - gl_in[0].gl_Position);
	vec4 v2 = normalize(gl_in[2].gl_Position - gl_in[0].gl_Position);

	normal = normalize(cross(v1.xyz, v2.xyz));

	// Find the altitudes (ha, hb and hc)
	float a = length(p1 - p2);
	float b = length(p2 - p0);
	float c = length(p1 - p0);
	float alpha = acos((b*b + c*c - a*a) / (2.0*b*c));
	float beta = acos((a*a + c*c - b*b) / (2.0*a*c));
	float ha = abs(c * sin(beta));
	float hb = abs(c * sin(alpha));
	float hc = abs(b * sin(alpha));

	// Projected direction
	
	//float d[4];
	// Send the triangle along with the edge distances
	gl_Position = p0;
	GEdgeDistance = vec3(ha, 0, 0);
	//uv = vec2(0, 0);
	//gDist = vec4(0, len[0] * angle[0], len[3] * angle[2], 0);
	EmitVertex();

	gl_Position = p1;
	GEdgeDistance = vec3(0, hb, 0);
	//uv = vec2(1, 0);
	//gDist = vec4(0, 0, len[1] * angle[1], len[0] * angle[3]);
	EmitVertex();

	gl_Position = p2;
	GEdgeDistance = vec3(0, 0, hc);
	//uv = vec2(1, 1);
	//gDist = vec4(len[1] * angle[0], 0, 0, len[2] * angle[2]);
	EmitVertex();
	
	/*for (int i = 0; i < gl_in.length; i++)
	{
		gl_Position = mvp * gl_in[i].gl_Position;
		EmitVertex();
	}*/
	//EndPrimitive();
}