#version 430
//uniform sampler2D modelTex;
/*out vec4 frag_color; // final colour of surface

void main()
{
	// final colour
	frag_color = vec4(1, 0, 0, 1.0);
}
*/

uniform vec3 white = vec3(1, 1, 1);

in vec3 normal;

out vec4 frag_color;


void main()
{

	// Mix the surface color with the line color
	frag_color = vec4((normal + white) * 0.5, 1.0);
}
