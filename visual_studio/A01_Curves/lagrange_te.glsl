#version 450
layout(isolines)in;
//uniform mat4 proj_mat;  // projection * view * model
uniform int degree;

void main()
{
	////
	// Lagrange Curve
	vec3 p[9];

	for (int i = 0; i <= degree; i++)
	{
		p[i] = gl_in[i].gl_Position.xyz;
	}
	float t = gl_TessCoord.x * degree;

	// Blossoming
	for (int j = 0; j < degree; j++)
	{
		for (int i = 0; i < degree - j; i++)
		{
			p[i] = p[i] * (i + j + 1 - t) / (j + 1)
				+ p[i + 1] * (t - i) / (j + 1);
		}
	}
	gl_Position = vec4(p[0], 1.0);

	/*
	float v[9];
	bool odd = (degree % 2 == 0);
	for (int i = 0; i <= degree / 2; i++)
	{
		v[i] = 1;
		for (int j = 0; j <= degree; j++)
		{
			if (i == j)
			{
				continue;
			}
			v[i] *= (i - j);
		}
		v[degree - i] = odd ? v[i] : -v[i];
	}

	float Lag[9];
	for (int i = 0; i <= degree; i++)
	{
		Lag[i] = 1.0;
		for (int j = 0; j <= degree; j++)
		{
			if (i == j)
			{
				continue;
			}
			Lag[i] *= (t - j);
		}
		Lag[i] /= v[i];
	}
	vec3 vp;
	for (int i = 0; i <= degree; i++)
	{
		vp += Lag[i] * p[i];
	}
	gl_Position = vec4(vp, 1.0);
	*/
}