layout( isolines ) in;
//uniform mat4 proj_matrix;  // projection * view * model
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
}