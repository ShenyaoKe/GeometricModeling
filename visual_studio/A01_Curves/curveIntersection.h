#ifndef __CURVEINTERSECTION__
#define __CURVEINTERSECTION__

#include "Accel/BBox.h"
const double curveEps = 0.001;

inline bool covered(const BBox &box0, const BBox& box1)
{
	if (box0.pMax.x < box1.pMin.x || box0.pMin.x > box1.pMax.x)
	{
		return false;
	}
	if (box0.pMax.y < box1.pMin.y || box0.pMin.y > box1.pMax.y)
	{
		return false;
	}
	return true;
}
inline bool covered(const BBox &bound,
	const Point3D* p0, const Point3D* p1)
{
	double lineMin, lineMax;

	lineMin = p0->x < p1->x ? p0->x : p1->x;
	lineMax = p0->x > p1->x ? p0->x : p1->x;
	if (lineMin > bound.pMax.x || lineMax < bound.pMin.x)
	{
		return false;
	}

	lineMin = p0->y < p1->y ? p0->y : p1->y;
	lineMax = p0->y > p1->y ? p0->y : p1->y;
	if (lineMin > bound.pMax.y || lineMax < bound.pMin.y)
	{
		return false;
	}

	if (p0->x != p1->x && p0->y != p1->y)
	{
		lineMin = INFINITE;
		lineMax = -INFINITE;
		vector<Point2D> pts;
		//Point2D np0(p0.x, p0->y), np1(p1->x, p1.y);
		pts.push_back(Point2D(bound.pMin.x - p0->x, bound.pMin.y - p0->y));
		pts.push_back(Point2D(bound.pMin.x - p0->x, bound.pMax.y - p0->y));
		pts.push_back(Point2D(bound.pMax.x - p0->x, bound.pMax.y - p0->y));
		pts.push_back(Point2D(bound.pMax.x - p0->x, bound.pMin.y - p0->y));
		Vector2D ny(p1->x - p0->x, p1->y - p0->y);
		double dist = ny.getLength();
		ny.normalize();

		bool isIntersect = false;
		for (int i = 0; i < 4; i++)
		{
			double len = pts[i] * ny;
			if (len < dist && len > 0)
			{
				isIntersect = true;
				break;
			}
		}
		if (!isIntersect)
		{
			return false;
		}

		Vector2D nx(ny.y, -ny.x);
		isIntersect = pts[0] * nx > 0;
		for (int i = 1; i < 4; i++)
		{
			if (isIntersect != (pts[i] * nx > 0))
			{
				return true;
			}
		}
		
	}
	return false;
}
// Line Box
/*
inline bool intersect(const vector<Point3D*> cv,
	const Point3D* p0, const Point3D* p1)
{
	//if bound !intersect line seg
	//return false
	//else subdiv
	return false;
}
// Line Line
inline bool intersect(const Point3D* p0, const Point3D* p1,
	const Point3D* q0, const Point3D* q1)
{
	if (true)
	{
	} 
	else
	{
	}
	return false;
}*/
// Line
inline bool isLine(const vector<Point3D*> &cv)
{
	Vector3D lineDir = Normalize(*cv.back() - *cv.front());
	swap(lineDir.x, lineDir.y);
	lineDir.y = -lineDir.y;

	for (int i = 1; i < cv.size() - 1; i++)
	{
		if (abs((*cv[i] - *cv.front()) * lineDir) > curveEps)
		{
			return false;
		}
	}
	return true;
}
#endif // __CURVEINTERSECTION__