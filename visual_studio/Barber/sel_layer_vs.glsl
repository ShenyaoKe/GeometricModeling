#version 430
in layout(location = 0) vec3 vp;

uniform mat4 view_matrix, proj_matrix;
uniform mat4 model_matrix = mat4(
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1);

void main()
{
	gl_Position = proj_matrix * view_matrix * model_matrix * vec4(vp, 1.0);
}