#version 430
//uniform sampler2D modelTex;
uniform int selid = 2;
uniform mat4 view_matrix;
in vec3 normal;
in vec2 TexCoord;  // From the geometry shader
in vec3 position_eye, normal_eye;

vec3 Kd = vec3(0.6, 0.8, 1);
vec3 La = vec3(0, 0, 0); // grey ambient colour
// fixed point light properties
vec3 light_position_world = vec3(0.0, 100.0, 2.0);


out vec4 frag_color; // final colour of surface

void main()
{
	int uid = gl_PrimitiveID;
	frag_color = vec4((uid >> 16) / 255.0, ((uid >> 8) & 0xFF) / 255.0,
		(uid & 0xFF) / 255.0, 1.0);

}

