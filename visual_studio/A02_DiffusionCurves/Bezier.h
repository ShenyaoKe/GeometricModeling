#pragma once
#ifndef __BEZIER__
#define __BEZIER__
#include <QColor>
#include "Core/Kaguya.h"
#include "Math/CGVector.h"
#include "Image/ColorData.h"
class Bezier
{
public:
	Bezier();
	~Bezier();
	void insertPoint(Point3D* pnt);
	void insertColor(ColorRGBA* color);
	void exportVBO(int &dg, int &size,
		float* &ptsVBO, float* &colorVBO);
public:
	int degree;
	vector<Point3D*> ctrlPts;// Control points
	vector<ColorRGBA*> colors;
};


#endif // __BEZIER__