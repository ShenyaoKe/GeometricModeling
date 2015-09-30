#version 430

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_colour;
//uniform mat4 proj_matrix;
//out vec3 colour;

void main() {
	//colour = vertex_colour;
	gl_Position = vec4(vertex_position.xy, -0.01, 1.0);
}
