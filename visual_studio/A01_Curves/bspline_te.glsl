#version 430
layout(isolines)in;
//uniform mat4 proj_matrix;  // projection * view * model
uniform int degree;

void main()
{
	////
	// B-Spline
	float t = gl_TessCoord.x + degree - 1;// n - 1 < t < n
	vec3 p[9];
	for (int i = 0; i <= degree; i++)
	{
		p[i] = gl_in[i].gl_Position.xyz;
	}
	for (int j = 0; j < degree; j++)
	{
		for (int i = 0; i < degree - j; i++)
		{
			p[i] = p[i] * (i + degree - t) / (degree - j)
				+ p[i + 1] * (t - i - j) / (degree - j);
		}
	}
	gl_Position = vec4(p[0], 1.0);
}