#ifndef HDS_FACE_H
#define HDS_FACE_H

#include "common.h"

#include <QVector3D>

class HDS_HalfEdge;
class HDS_Vertex;
class HDS_Mesh;

class HDS_Face
{
private:
	static size_t uid;

public:
	static void resetIndex() { uid = 0; }
	static size_t assignIndex() { return uid++; }
	static float faceAngle(const HDS_Face* f0, const HDS_Face* f1);

	HDS_Face();
	~HDS_Face();

	HDS_Face(const HDS_Face &other);
	HDS_Face operator=(const HDS_Face &other);
	
	void updateVerts();
	set<HDS_Face *> connectedFaces();
	set<HDS_Face *> linkedFaces();
	QVector3D center() const;
	vector<HDS_Vertex*> corners() const;
	QVector3D computeNormal();

private:
public:
	QVector3D n;		// Geometric face normal
	HDS_HalfEdge *he;

	int vnum;				// Number of vertices
	vector<HDS_Vertex*> v;	// actual vertices

	unordered_map<HDS_Vertex*, QVector3D*> snMap;
	unordered_map<HDS_Vertex*, QVector2D*> suvMap;

	int index;
};

#endif // HDS_FACE_H
