#version 450
// extension should be core -- commented out
// replaced gl_VerticesIn with gl_in.length ()
//#extension GL_EXT_geometry_shader4 : enable

layout (points) in;
// convert to points, line_strip, or triangle_strip
layout(line_strip, max_vertices = 5) out;
uniform mat4 proj_mat;
uniform vec2 win_size;
uniform float pointsize = 4;// 0.04;


void main () {
	for(int i = 0; i < gl_in.length (); i++) {
		// use original point as first point in triangle strip
		vec4 cur_pos = gl_in[i].gl_Position;
		cur_pos.x -= pointsize;
		cur_pos.y -= pointsize;

		gl_Position = proj_mat * cur_pos;
		EmitVertex();
		// create another point relative to the previous
		cur_pos.y += pointsize;
		gl_Position = proj_mat * cur_pos;
		//f_colour = colour[0];
		EmitVertex();
		// create another point relative to the previous
		cur_pos.x += pointsize;
		gl_Position = proj_mat * cur_pos;
		//f_colour = colour[0];
		EmitVertex();
		// create another point relative to the previous
		cur_pos.y -= pointsize;
		gl_Position = proj_mat * cur_pos;
		//f_colour = colour[0];
		EmitVertex();
		// create another point relative to the previous
		cur_pos.x -= pointsize;
		gl_Position = proj_mat * cur_pos;
		EmitVertex();
	}
}
