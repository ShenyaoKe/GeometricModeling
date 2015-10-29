#include "Bezier.h"


Bezier::Bezier()
	: degree(3)
{
	colors[0] = QColor(102, 224, 143);
	colors[1] = QColor(204, 26, 61);
	colors[2] = QColor(77, 127, 140);
	colors[3] = QColor(153, 77, 204);
}


Bezier::~Bezier()
{
	for (auto pnt : ctrlPts)
	{
		delete pnt;
	}
	ctrlPts.clear();
}

void Bezier::insertPoint(QVector3D* pnt)
{
	//QVector3D* point = new QVector3D(*pnt);
	ctrlPts.push_back(pnt);
}

void Bezier::insertColor(const QColor* clrs)
{
	for (int i = 0; i < 4; i++)
	{
		colors[i] = clrs[i];
	}
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
		colorVBO = new GLfloat[12];

		for (int i = 0; i < size; i++)
		{
			//auto curPt = ctrlPts[i];
			ptsVBO[i * 3] = ctrlPts[i]->x();
			ptsVBO[i * 3 + 1] = ctrlPts[i]->y();
			ptsVBO[i * 3 + 2] = ctrlPts[i]->z();

			/**/
		}
		for (int i = 0; i < 4; i++)
		{
			colorVBO[i * 3] = static_cast<GLfloat>(colors[i].redF());
			colorVBO[i * 3 + 1] = static_cast<GLfloat>(colors[i].greenF());
			colorVBO[i * 3 + 2] = static_cast<GLfloat>(colors[i].blueF());
		}
	}
}
