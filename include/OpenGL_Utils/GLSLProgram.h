#pragma once
#ifndef __GLSLPROGRAM__
#define __GLSLPROGRAM__

#include <GL/glew.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

const GLenum shader_type[5] = {
	GL_VERTEX_SHADER,
	GL_FRAGMENT_SHADER,
	GL_GEOMETRY_SHADER,
	GL_TESS_CONTROL_SHADER,
	GL_TESS_EVALUATION_SHADER
};

class GLSLProgram
{
public:
	GLSLProgram();
	GLSLProgram(char *vert, char* frag = nullptr, char* geom = nullptr,
		char* tcs = nullptr, char* tes = nullptr);
	~GLSLProgram();

	bool create(char *vert, char* frag = nullptr, char* geom = nullptr,
		char* tcs = nullptr, char* tes = nullptr);
	bool use_program() const;

	int getUniformLocation(const char *name) const;
	
private:
	bool read_shader_file(const char *file_name, char* &shader_str) const;
	bool create_shader(const char *file_name, GLuint &shader, GLenum type);
	bool create_program();

	GLuint shaders[5];
	GLuint program;
};

#endif // __GLSLPROGRAM__