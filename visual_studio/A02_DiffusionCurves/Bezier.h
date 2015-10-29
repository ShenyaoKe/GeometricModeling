#pragma once
#ifndef __BEZIER__
#define __BEZIER__

#include <QColor>
#include <QVector3D>

//#include "Core/Kaguya.h"
#include "common.h"
#include "GL/glew.h"
//#include "Math/CGVector.h"
//#include "Image/ColorData.h"

//typedef QVector3D Point3D;
class Bezier
{
public:
	Bezier();
	~Bezier();
	void insertPoint(QVector3D* pnt);
	void insertColor(const QColor* clrs);
	void exportVBO(int &dg, int &size,
		GLfloat* &ptsVBO, GLfloat* &colorVBO);
public:
	int degree;
	vector<QVector3D*> ctrlPts;// Control points
	QColor colors[4];
};


#endif // __BEZIER__