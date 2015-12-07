#version 430
/*in vec3 normal;*/
in vec2 uv;
out vec4 frag_color;
void main ()
{
	frag_color = vec4(0.4, 0.88, 0.56, 1 - uv.x);
}