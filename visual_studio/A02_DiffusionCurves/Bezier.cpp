#include "Bezier.h"


Bezier::Bezier()
	: degree(3)
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

void Bezier::insertPoint(QVector3D* pnt)
{
	//QVector3D* point = new QVector3D(*pnt);
	ctrlPts.push_back(pnt);
}

void Bezier::insertColor(QColor* color)
{
	colors.push_back(color);
}

void Bezier::exportVBO(int &dg, int &size, GLfloat* &ptsVBO, GLfloat* &colorVBO)
{
	dg = degree;
	size = ctrlPts.size();
	if (size > 0)
	{
		delete[] ptsVBO;
		ptsVBO = new GLfloat[size * 3];
		delete[] colorVBO;
		colorVBO = new GLfloat[size * 3];

		for (int i = 0; i < size; i++)
		{
			//auto curPt = ctrlPts[i];
			ptsVBO[i * 3] = ctrlPts[i]->x();
			ptsVBO[i * 3 + 1] = ctrlPts[i]->y();
			ptsVBO[i * 3 + 2] = ctrlPts[i]->z();

			/*colorVBO[i * 3] = static_cast<float>(colors[i]->r);
			colorVBO[i * 3 + 1] = static_cast<float>(colors[i]->g);
			colorVBO[i * 3 + 2] = static_cast<float>(colors[i]->b);*/
		}
	}
}
