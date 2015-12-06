#version 430
//uniform sampler2D modelTex;
uniform int sel_id;
uniform mat4 view_matrix;
//in vec3 norm;
in vec3 pos_eye, norm_eye;
in float cosTheta;


uniform vec3 Kd = vec3(0.6, 0.72, 0.8);
uniform vec3 La = vec3(0.1, 0.1, 0.1); // grey ambient colour
uniform vec3 white = vec3(1, 1, 1);
// fixed point light properties
uniform vec3 light_pos = vec3(0, 0, 0);


out vec4 frag_color; // final colour of surface

void main()
{
	// ambient intensity
	// diffuse intensity
	// raise light position to eye space
	vec3 dir_lt = normalize(light_pos - pos_eye);
	float cosTheta = abs(dot(dir_lt, norm_eye));
	vec3 Id = mix(La, Kd, cosTheta); // final diffuse intensity
	
	// final colour
	frag_color = vec4(Id, 1.0);

	if (gl_PrimitiveID == sel_id)
	{
		frag_color = mix(frag_color, vec4(1, 0.6, 0.35, 1), 0.5);
	}

}

