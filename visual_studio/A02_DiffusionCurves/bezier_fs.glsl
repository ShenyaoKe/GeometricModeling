#version 430
uniform vec3 f_lf_color;
uniform vec3 f_rt_color;
uniform vec3 e_lf_color;
uniform vec3 e_rt_color;

in vec2 uv_gs;

out vec4 frag_color;

void main ()
{
	//vec2 uv_gs = uv_fs;
	float u = uv_gs.x;
	float v = uv_gs.y;
	if (v < 0.5)
	{
		frag_color = vec4(u * f_lf_color + (1 - u) * f_rt_color, 1.0);
	}
	else
	{
		frag_color = vec4(u * e_lf_color + (1 - u) * e_rt_color, 1.0);
	}
}