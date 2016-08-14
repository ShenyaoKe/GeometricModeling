#version 440
layout(isolines) in;
uniform int degree = 2;
patch in float dist[4];

void main()
{
	float s[4];
	s[0] = 0;
	s[1] = dist[1] + s[0];
	s[2] = dist[2] + s[1];
	s[3] = dist[3] + s[2];
	// Catmull-Rom Spline
	// s[degree - 1] < t < s[degree]
	float t = (s[2] - s[1]) * gl_TessCoord.x + s[1];

	vec3 p[4] = { gl_in[0].gl_Position.xyz,
				  gl_in[1].gl_Position.xyz,
				  gl_in[2].gl_Position.xyz,
				  gl_in[3].gl_Position.xyz };
	
	// Lagrange
	p[0] = (p[0]*(s[1] - t) + p[1] * (t - s[0])) / (s[1] - s[0]);
	p[1] = (p[1]*(s[2] - t) + p[2] * (t - s[1])) / (s[2] - s[1]);
	p[2] = (p[2]*(s[3] - t) + p[3] * (t - s[2])) / (s[3] - s[2]);

	p[0] = (p[0]*(s[2] - t) + p[1] * (t - s[0])) / (s[2] - s[0]);
	p[1] = (p[1]*(s[3] - t) + p[2] * (t - s[1])) / (s[3] - s[1]);

	//de Boor
	p[0] = (p[0] * (s[2] - t) + p[1] * (t - s[1])) / (s[2] - s[1]);
	
	gl_Position = vec4(p[0], 1.0);
}