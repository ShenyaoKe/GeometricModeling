#include "Bezier.h"


Bezier::Bezier()
{
}


Bezier::~Bezier()
{
	for (auto pnt : ctrlPts)
	{
		delete pnt;
	}
	ctrlPts.clear();
	for (auto color : colors)
	{
		delete color;
	}
	colors.clear();
}

void Bezier::insertPoint(Point3D* pnt)
{
	ctrlPts.push_back(pnt);
}

void Bezier::insertColor(ColorRGBA* color)
{
	colors.push_back(color);
}

void Bezier::exportVBO(int &dg, int &size, float* &ptsVBO, float* &colorVBO)
{
	dg = degree;
	size = ctrlPts.size();
	if (size > 0)
	{
		delete[] ptsVBO;
		ptsVBO = new float[size * 3];
		delete[] colorVBO;
		colorVBO = new float[size * 3];

		for (int i = 0; i < size; i++)
		{
			ptsVBO[i * 3] = static_cast<float>(ctrlPts[i]->x);
			ptsVBO[i * 3 + 1] = static_cast<float>(ctrlPts[i]->y);
			ptsVBO[i * 3 + 2] = static_cast<float>(ctrlPts[i]->z);

			colorVBO[i * 3] = static_cast<float>(colors[i]->r);
			colorVBO[i * 3 + 1] = static_cast<float>(colors[i]->g);
			colorVBO[i * 3 + 2] = static_cast<float>(colors[i]->b);
		}
	}
}
