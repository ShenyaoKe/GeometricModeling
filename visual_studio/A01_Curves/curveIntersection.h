#ifndef __CURVEINTERSECTION__
#define __CURVEINTERSECTION__

#include "Accel/Bounds.h"
#include "Math/Geometry.h"
const double curveEps = 0.001;

inline bool covered(const Bounds2f &box0, const Bounds2f& box1)
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
inline bool covered(const Bounds2f &bound,
	const Point2f* p0, const Point2f* p1)
{
	double lineMin, lineMax;

	lineMin = p0->x < p1->x ? p0->x : p1->x;
	lineMax = p0->x > p1->x ? p0->x : p1->x;
	if (lineMin > bound.pMax.x || lineMax < bound.pMin.x) return false;

	lineMin = p0->y < p1->y ? p0->y : p1->y;
	lineMax = p0->y > p1->y ? p0->y : p1->y;
	if (lineMin > bound.pMax.y || lineMax < bound.pMin.y) return false;

	if (p0->x != p1->x && p0->y != p1->y)
	{
		lineMin = std::numeric_limits<double>::infinity();
		lineMax = -std::numeric_limits<double>::infinity();
		vector<Vector2f> pts;
		//Point2f np0(p0.x, p0->y), np1(p1->x, p1.y);
		pts.push_back(Vector2f(bound.pMin.x - p0->x, bound.pMin.y - p0->y));
		pts.push_back(Vector2f(bound.pMin.x - p0->x, bound.pMax.y - p0->y));
		pts.push_back(Vector2f(bound.pMax.x - p0->x, bound.pMax.y - p0->y));
		pts.push_back(Vector2f(bound.pMax.x - p0->x, bound.pMin.y - p0->y));
		Vector2f ny(p1->x - p0->x, p1->y - p0->y);
		double dist = ny.length();
		ny.normalize();

		bool isIntersect = false;
		for (int i = 0; i < 4; i++)
		{
			double len = Dot(pts[i], ny);
			if (len < dist && len > 0)
			{
				isIntersect = true;
				break;
			}
		}
		if (!isIntersect) return false;

		Vector2f nx(ny.y, -ny.x);
		isIntersect = Dot(pts[0], nx) > 0;
		for (int i = 1; i < 4; i++)
		{
			if (isIntersect != (Dot(pts[i], nx) > 0))
			{
				return true;
			}
		}
		
	}
	return false;
}
// Line Box
/*
inline bool intersect(const vector<Point2f*> cv,
	const Point2f* p0, const Point2f* p1)
{
	//if bound !intersect line seg
	//return false
	//else subdiv
	return false;
}
// Line Line
inline bool intersect(const Point2f* p0, const Point2f* p1,
	const Point2f* q0, const Point2f* q1)
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
inline bool isLine(const vector<Point2f*> &cv)
{
	
	auto lineDir = Normalize(*cv.back() - *cv.front());
	swap(lineDir.x, lineDir.y);
	lineDir.y = -lineDir.y;

	for (int i = 1; i < cv.size() - 1; i++)
	{
		if (abs(Dot(*cv[i] - *cv.front(), lineDir)) > curveEps)
		{
			return false;
		}
	}
	return true;
}

inline void subdivBezier(const vector<Point2f*> &cvs, vector<Point2f*> &lf_cvs, vector<Point2f*> &rt_cvs)
{
	vector<Point2f*> tmpCVS(cvs);
	lf_cvs.push_back(cvs.front());
	rt_cvs.push_back(cvs.back());
	for (int j = cvs.size() - 1; j > 0; j--)
	{
		for (int i = 0; i < j; i++)
		{
			Point2f* curPt = new Point2f((*tmpCVS[i] + *tmpCVS[i + 1]) * 0.5);
			tmpCVS[i] = curPt;
		}
		lf_cvs.push_back(tmpCVS[0]);
		rt_cvs.push_back(tmpCVS[j - 1]);
	}
}
#endif // __CURVEINTERSECTION__