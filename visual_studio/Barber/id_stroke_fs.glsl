#version 430

out vec4 frag_color; // final colour of surface

uniform int strokeid = 0;
uniform int mode = 0;
void main()
{
	
	if (mode != 1)
	{
		frag_color = vec4(0, 0, 0, 0);
	}
	else
	{
		frag_color = vec4((strokeid >> 16) / 255.0, ((strokeid >> 8) & 0xFF) / 255.0,
			(strokeid & 0xFF) / 255.0, 1.0);
	}
	
}

