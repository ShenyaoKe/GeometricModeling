#version 430
in layout(location = 0) vec3 vp;
in layout(location = 1) vec3 vn;
in layout(location = 2) vec2 uv;

uniform mat4 view_matrix, proj_matrix;

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