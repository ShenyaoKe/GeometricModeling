#pragma once
#include <GL/glew.h>
#include <cstdio>
#include <cstdlib>

class oglVBO
{
public:
	oglVBO();
	~oglVBO();

	//void CollapseCommonVertices(bool collapse);
	void Draw();
	void Begin(GLenum type);
	void Color4fv(GLdouble *rgba);
	void End();
	void Normal3fv(GLdouble *nxyz);
	void TexCoord2fv(GLdouble *st);
	void Vertex2fv(GLdouble *xy);
	void Vertex3fv(GLdouble *xyz);
	//void Print(char *str = "", FILE *out = stderr);
	//void RestartPrimitive();
	void SetTol(float tol);
private:
	GLdouble *pts;
	GLdouble *normal;
	GLdouble *vcolor;
	GLdouble *texcoord;
	GLuint num;
};