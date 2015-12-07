#version 440
in layout(location = 0) vec3 vp;

//uniform mat4 view_matrix, proj_matrix;

void main()
{
	gl_Position = vec4(vp, 1.0);
}