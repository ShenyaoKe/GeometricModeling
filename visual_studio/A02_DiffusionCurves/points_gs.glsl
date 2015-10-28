#version 430
// extension should be core -- commented out
// replaced gl_VerticesIn with gl_in.length ()
//#extension GL_EXT_geometry_shader4 : enable

layout (points) in;
// convert to points, line_strip, or triangle_strip
layout(line_strip, max_vertices = 5) out;
uniform mat4 proj_matrix;

uniform float pointsize;// 0.04;

// NB: in and out pass-through vertex->fragment variables must go here if used
//in vec3 colour[];
//out vec3 f_colour;

void main ()
{
	for(int i = 0; i < gl_in.length (); i++)
	{
		// use original point as first point in triangle strip
		vec4 cur_pos = gl_in[i].gl_Position;
		cur_pos.x -= pointsize * 0.5;
		cur_pos.y -= pointsize * 0.5;
		gl_Position = proj_matrix * cur_pos;
		// output pass-through data to go to fragment-shader (colour)
		//f_colour = colour[0];
		// finalise first vertex
		EmitVertex();
		// create another point relative to the previous
		cur_pos.y += pointsize;
		gl_Position = proj_matrix * cur_pos;
		//f_colour = colour[0];
		EmitVertex();
		// create another point relative to the previous
		cur_pos.x += pointsize;
		gl_Position = proj_matrix * cur_pos;
		//f_colour = colour[0];
		EmitVertex();
		// create another point relative to the previous
		cur_pos.y -= pointsize;
		gl_Position = proj_matrix * cur_pos;
		//f_colour = colour[0];
		EmitVertex();
		// create another point relative to the previous
		cur_pos.x -= pointsize;
		gl_Position = proj_matrix * cur_pos;
		EmitVertex();
	}
}
