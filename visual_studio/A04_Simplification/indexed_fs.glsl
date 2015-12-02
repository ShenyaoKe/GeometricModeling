#version 430

uniform vec3 white = vec3(1, 1, 1);

in vec3 normal;
noperspective in vec3 GEdgeDistance;
out vec4 frag_color;


void main()
{
	/*
	float d = min(min(GEdgeDistance.x, GEdgeDistance.y), GEdgeDistance.z);
	if (d < 0.1)
	{
		d = 0;
	}
	else
	{
		d = 1;
	}
	frag_color = mix(vec4(0, 1, 0.4, 1), vec4((normal + white) * 0.5, 1.0), d);
	*/
	frag_color = vec4((normal + white) * 0.5, 1.0);
}
