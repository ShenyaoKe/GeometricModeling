#ifndef HDS_EDGE_H
#define HDS_EDGE_H

#include "hds_face.h"
#include "hds_vertex.h"

class HDS_Face;
class HDS_Vertex;

class HDS_HalfEdge
{
private:
	static size_t uid;

public:
	static void resetIndex() { uid = 0; }
	static size_t assignIndex() { return uid++; }

	HDS_HalfEdge();
	~HDS_HalfEdge();
	HDS_HalfEdge(HDS_Vertex *v, HDS_Face *f);
	HDS_HalfEdge(const HDS_HalfEdge& other);

	static void connectEdges(HDS_HalfEdge* he, HDS_HalfEdge* hef);
	float length() const;
public:
	HDS_Face *f;
	HDS_Vertex *v;
	HDS_HalfEdge *prev, *next, *flip;

	int index;

	//float restlength;
};

#endif // HDS_EDGE_H
