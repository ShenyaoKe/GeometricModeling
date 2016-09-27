#version 430
layout(location = 0) in vec3 vp;

//uniform mat4 view_matrix, proj_matrix;
//out mat4 mvp;

void main()
{
	//mvp = proj_matrix * view_matrix;
	gl_Position = vec4(vp, 1.0);
	//gl_Position = vec4(vp, 1.0);
}