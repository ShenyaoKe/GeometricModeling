#version 430

layout(location = 0) in vec2 vp;
//uniform mat4 proj_matrix;
//out vec3 colour;

void main() {
	//colour = vertex_colour;
	gl_Position = vec4(vp, -1.0, 1.0);
}
