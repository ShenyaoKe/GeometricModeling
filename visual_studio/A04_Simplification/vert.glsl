#version 430
layout(location = 0) in vec3 vp;
layout(location = 1) in vec3 vn;
layout(location = 2) in vec2 uv;

uniform mat4 view_matrix, proj_matrix;
uniform mat4 model_matrix = mat4(
								1, 0, 0, 0,
								0, 1, 0, 0,
								0, 0, 1, 0,
								0, 0, 0, 1);

out vec3 normal;
out vec2 TexCoord;

out vec3 position_eye, normal_eye;

void main()
{
	normal = vn;
	TexCoord = uv;
	//mat4 model_matrix;
	position_eye = vec3(view_matrix * model_matrix * vec4(vp, 1.0));
	normal_eye = normalize(vec3(view_matrix * model_matrix * vec4(vn, 0.0)));
	gl_Position = proj_matrix * view_matrix * model_matrix * vec4(vp, 1.0);
}