#version 430

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;

out vec3 normal;
out vec2 TexCoord;
out vec4 gDist;
out vec3 position_eye, normal_eye;

uniform mat4 view_matrix, proj_matrix;



void main()
{
	mat4 mvp = proj_matrix * view_matrix;
	// Transform each vertex into viewport space
	vec4 p0 = mvp * gl_in[0].gl_Position;
	vec4 p1 = mvp * gl_in[1].gl_Position;
	vec4 p2 = mvp * gl_in[2].gl_Position;
	vec4 p3 = mvp * gl_in[3].gl_Position;

	vec4 v1 = normalize(gl_in[2].gl_Position - gl_in[0].gl_Position);
	vec4 v2 = normalize(gl_in[3].gl_Position - gl_in[1].gl_Position);

	normal = normalize(cross(v1.xyz, v2.xyz));
	normal_eye = normalize(view_matrix * vec4(normal, 0)).xyz;

	// Projected direction
	vec2 e0 = (p1 - p0).xy;
	vec2 e1 = (p2 - p1).xy;
	vec2 e2 = (p3 - p2).xy;
	vec2 e3 = (p0 - p3).xy;
	float len[4];
	len[0] = length(e0); len[1] = length(e1); len[2] = length(e2); len[3] = length(e3);
	e0 = normalize(e0); e1 = normalize(e1); e2 = normalize(e2); e3 = normalize(e3);
	float angle[4];
	angle[0] = dot(e1, e0); angle[0] = sqrt(1 - angle[0] * angle[0]);
	angle[1] = dot(e2, e1); angle[1] = sqrt(1 - angle[1] * angle[1]);
	angle[2] = dot(e3, e2); angle[2] = sqrt(1 - angle[2] * angle[2]);
	angle[3] = dot(e0, e3); angle[3] = sqrt(1 - angle[3] * angle[3]);

	//float d[4];
	// Send the triangle along with the edge distances
	gl_PrimitiveID = gl_PrimitiveIDIn; 
	
	gl_Position = p0;
	position_eye = (view_matrix * gl_in[0].gl_Position).xyz;
	TexCoord = vec2(0, 0);
	EmitVertex();

	gl_Position = p1;
	position_eye = (view_matrix * gl_in[1].gl_Position).xyz;
	TexCoord = vec2(1, 0);
	EmitVertex();

	gl_Position = p3;
	position_eye = (view_matrix * gl_in[3].gl_Position).xyz;
	TexCoord = vec2(0, 1);
	EmitVertex();

	gl_Position = p2;
	position_eye = (view_matrix * gl_in[2].gl_Position).xyz;
	TexCoord = vec2(1, 1);
	EmitVertex();
}