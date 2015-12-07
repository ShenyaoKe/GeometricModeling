#version 440
layout(isolines) in;

uniform mat4 view_matrix, proj_matrix;

uniform int degree = 2;
uniform int n_layer = 2;
patch in int vertCount;
out vec2 uv;
vec4 root, tip;
vec4 lerp(int ly, int offset, float u)
{
	int sID = ly * 4;
	return mix(root, tip, u);
}
void main()
{
	// Catmull-Rom Spline
	//float t = gl_TessCoord.x + degree - 1;
	//vec3 p[gl_MaxPatchVertices];

	float u = gl_TessCoord.x;// (n_layer - 1) * t;
	float v = gl_TessCoord.y;
	uv = gl_TessCoord.xy;
	float s = sqrt(v);
	float t = sqrt(1 - v);
	int tipLyLoc = vertCount - 4;
	root = mix(
		mix(gl_in[0].gl_Position, gl_in[1].gl_Position, s),
		mix(gl_in[2].gl_Position, gl_in[3].gl_Position, 1 - s),
		t);
	tip = mix(
		mix(gl_in[tipLyLoc].gl_Position, gl_in[tipLyLoc + 1].gl_Position, s),
		mix(gl_in[tipLyLoc + 2].gl_Position, gl_in[tipLyLoc + 3].gl_Position, 1 - s),
		t);
	int offset = int(gl_TessCoord.y * 2.5);
	int ly = 0;// int(u);
	gl_Position =  lerp(ly, offset, u);
	gl_Position = proj_matrix * view_matrix * gl_Position;
	// Catmull-Rom Blossom
	/*for (int i = 0; i < 2 * degree; i++)
	{
		p[i] = gl_in[i].gl_Position.xyz;
	}
	// Lagrange
	for (int j = 0; j < degree; j++)
	{
		for (int i = 0; i < 2 * degree - j - 1; i++)
		{
			p[i] = p[i] * (i + j + 1 - t) / (j + 1)
				+ p[i + 1] * (t - i) / (j + 1);
		}
	}
	//de Boor
	for (int j = 0; j < degree - 1; j++)
	{
		int dem = degree - j - 1;
		for (int i = 0; i < dem; i++)
		{
			p[i] = p[i] * (i + degree - t) / dem
				+ p[i + 1] * (t - i - j - 1) / dem;
		}
	}*/
	//gl_Position = vec4(p[0], 1.0);
}