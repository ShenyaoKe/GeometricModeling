#version 430
//uniform sampler2D modelTex;
uniform int sel_id;
uniform mat4 view_matrix;
in vec3 normal;
in vec2 TexCoord;  // From the geometry shader
in vec3 position_eye, normal_eye;


vec3 Kd = vec3(0.6, 0.8, 1);
vec3 La = vec3(0, 0, 0); // grey ambient colour
// fixed point light properties
vec3 light_pos_world = vec3(0.0, 100.0, 200.0);


out vec4 frag_color; // final colour of surface

void main()
{
	// ambient intensity
	// diffuse intensity
	// raise light position to eye space
	vec3 light_pos_eye = light_pos_world;
	vec3 dist_to_light_eye = light_pos_eye - position_eye;
	vec3 dir_to_light_eye = normalize(dist_to_light_eye);
	float dot_prod = dot(dir_to_light_eye, normal_eye);
	dot_prod = (dot_prod + 1.0) / 2.0;
	vec3 Id = mix(La, Kd, dot_prod); // final diffuse intensity
	
	// final colour
	frag_color = vec4(Id, 1.0);
	//frag_color = vec4(0.1 * gl_PrimitiveID, 0, 0, 1.0);

	if (gl_PrimitiveID == sel_id)
	{
		frag_color = mix(frag_color, vec4(1, 0.6, 0.35, 1), 0.5);
	}

}

