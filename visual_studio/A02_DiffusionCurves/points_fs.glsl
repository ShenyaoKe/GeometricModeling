#version 430

//in vec3 f_colour;
uniform vec3 pointcolor;
out vec4 frag_colour;

void main() {
	frag_colour = vec4(pointcolor, 1);// vec4(f_colour, 1.0);
}