#version 430
//uniform sampler2D modelTex;
uniform mat4 view_matrix;
in vec3 pos_eye, norm_eye;

uniform vec3 color;
uniform vec3 Kd;
uniform vec3 La = vec3(0, 0, 0); // grey ambient colour
// fixed point light properties
uniform vec3 light_pos = vec3(0, 0, 0);

uniform bool selected = false;

out vec4 frag_color; // final colour of surface

void main()
{
	// ambient intensity
	// diffuse intensity
	// raise light position to eye space
	vec3 dir_lt = normalize(light_pos - pos_eye);
	float cosTheta = dot(dir_lt, norm_eye);
	cosTheta = abs(cosTheta);
	vec3 Id = mix(La, Kd, cosTheta); // final diffuse intensity

	frag_color = vec4(color, 1.0);
	// final colour
	if (selected)
	{
		frag_color = mix(frag_color, vec4(1, 0.6, 0.35, 1), 0.5);
	}
}

