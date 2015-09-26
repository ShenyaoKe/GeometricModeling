#version 430
layout(isolines) in;
uniform int degree;

void main()
{
	////
	// Bezier Curve
    // The tessellation u coordinate
    float u = gl_TessCoord.x;

	// The patch vertices (control points)
	vec3 p[9];
	for (int i = 0; i <= degree; i++)
	{
		p[i] = gl_in[i].gl_Position.xyz;
	}
	// Blossoming
	for (int j = degree; j >= 0; j--)
	{
		for (int i = 0; i < j; i++)
		{
			p[i] = p[i] * (1.0 - u) + p[i + 1] * u;
		}
	}
	/*
    float u1 = (1.0 - u);
    float u2 = u * u;

    // Bernstein polynomials evaluated at u
	float b[9];

    b[3] = u2 * u;
    b[2] = 3.0 * u2 * u1;
    b[1] = 3.0 * u * u1 * u1;
    b[0] = u1 * u1 * u1;
	
    // Cubic Bezier interpolation
	vec3 vp;// = vec3(0, 0, 0);
	for (int i = 0; i <= degree; i++)
	{
		vp += p[i] * b[i];
	}
	//vec3 vp = p[0] * b[0] + p[1] * b[1] + p[2] * b[2] + p[3] * b[3];

	gl_Position = vec4(vp, 1.0);*/
	gl_Position = vec4(p[0], 1.0);
}