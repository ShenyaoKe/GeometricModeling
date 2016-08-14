#version 450
layout(location = 0) in vec2 vp;
/*uniform mat4 model_matrix; // ModelToWorld matrix
uniform mat4 view_matrix; // WorldToCamera matrix
uniform mat4 proj_mat; // Projection matrix
*/
uniform mat4 proj_mat;

void main () {
	gl_Position = proj_mat * vec4(vp, 0, 1.0);
}