#version 430
in vec3 vp;
/*uniform mat4 model_matrix; // ModelToWorld matrix
uniform mat4 view_matrix; // WorldToCamera matrix
uniform mat4 proj_matrix; // Projection matrix
*/

void main () {
	gl_Position = vec4(vp, 1.0);
}