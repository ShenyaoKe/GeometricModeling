#ifndef HDS_VERTEX_H
#define HDS_VERTEX_H

#include "common.h"

#include <QVector2D>
#include <QVector3D>

class HDS_HalfEdge;

class HDS_Vertex
{
private:
	static size_t uid;

public:
	static void resetIndex() { uid = 0; }
	static size_t assignIndex() { return uid++; }

	HDS_Vertex();
	HDS_Vertex(const QVector3D &p);
	HDS_Vertex(const HDS_Vertex &other);
	~HDS_Vertex();

	void computeNormal();
	vector<HDS_Vertex *> neighbors() const;

	qreal x() { return pos.x(); }
	qreal y() { return pos.y(); }
	qreal z() { return pos.z(); }

public:
	QVector3D pos, pre_pos;
	QVector3D normal;	// geometric normal
	//QVector2D texcoord;
	HDS_HalfEdge *he;

	QVector3D vel, pre_vel;
	QVector3D in_force;	// Internal force on vertex

	int index;
};

#endif // HDS_VERTEX_H
