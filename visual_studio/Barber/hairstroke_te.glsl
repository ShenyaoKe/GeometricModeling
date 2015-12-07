#version 440
layout(isolines) in;
//uniform mat4 proj_matrix;  // projection * view * model
uniform int degree;

void main()
{
	// Catmull-Rom Spline
	float t = gl_TessCoord.x + degree - 1;
	vec3 p[gl_MaxPatchVertices];
	for (int i = 0; i < 2 * degree; i++)
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
	}
	gl_Position = vec4(p[0], 1.0);
}