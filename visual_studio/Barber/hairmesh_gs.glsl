#version 430

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;

out vec3 normal;
out vec2 TexCoord;
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