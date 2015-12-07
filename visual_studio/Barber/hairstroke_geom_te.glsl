#version 430
layout(isolines, equal_spacing) in;
uniform int degree;
out vec2 uv_te;
//out vec4 T_te;
out vec4 B_te;

vec3 BezierBlossom(float u)
{
	// The patch vertices (control points)
	vec3 p[9];
	for (int i = 0; i <= degree; i++)
	{
		p[i] = gl_in[i].gl_Position.xyz;
	}
	// Blossoming
	for (int j = degree; j > 0; j--)
	{
		for (int i = 0; i < j; i++)
		{
			p[i] = p[i] * (1.0 - u) + p[i + 1] * u;
		}
	}
	return p[0];
}

void main()
{
	////
	// Bezier Curve
    // The tessellation u coordinate
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	// The patch vertices (control points)
	vec3 curPos[3];
	curPos[0] = BezierBlossom(u - 0.001);
	curPos[1] = BezierBlossom(u);
	curPos[2] = BezierBlossom(u + 0.001);
	gl_Position = vec4(curPos[1], 1.0);// +vec4(0, v*0.1, 0, 0);
	vec3 T_te = normalize(curPos[2] - curPos[0]);
	B_te = vec4(cross(vec3(0, 0, 1), T_te), 0.0);
	uv_te = gl_TessCoord.xy;
}