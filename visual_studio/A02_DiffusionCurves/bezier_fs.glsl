#version 430
/*in vec3 normal;*/
uniform vec3 color0 = vec3(0.4, 0.88, 0.56);
uniform vec3 color1 = vec3(0.8, 0.1, 0.24);
uniform vec3 color2 = vec3(0.3, 0.5, 0.55);
uniform vec3 color3 = vec3(0.6, 0.3, 0.8);
in vec2 uv_gs;
out vec4 frag_color;
void main ()
{
	//vec2 uv_gs = uv_fs;
	float u = uv_gs.x;
	float v = uv_gs.y;
	if (v < 0.5)
	{
		frag_color = vec4(u * color0 + (1 - u) * color1, 1.0);
	}
	else
	{
		frag_color = vec4(u * color2 + (1 - u) * color3, 1.0);
	}
	//frag_color = vec4(0.4,0.88,0.56,1.0);
}