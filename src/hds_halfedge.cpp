#include "hds_halfedge.h"

size_t HDS_HalfEdge::uid = 0;

HDS_HalfEdge::HDS_HalfEdge()
{
	index = -1;
	f = nullptr;
	v = nullptr;
	flip = nullptr;
	prev = nullptr;
	next = nullptr;
}


HDS_HalfEdge::HDS_HalfEdge(const HDS_HalfEdge &other)
	: index(other.index)
{
	//index = other.index;
	f = nullptr;
	v = nullptr;
	flip = nullptr;
	prev = nullptr;
	next = nullptr;
}

HDS_HalfEdge::HDS_HalfEdge(HDS_Vertex *v, HDS_Face *f)
{
	this->f = f;
	this->v = v;
}

void HDS_HalfEdge::connectEdges(HDS_HalfEdge* he, HDS_HalfEdge* hef)
{
	if (he == nullptr || hef == nullptr)
	{
		return;
	}
	he->flip = hef;
	hef->flip = he;
}

float HDS_HalfEdge::length() const
{
	return (v->pos - flip->v->pos).length();
}

HDS_HalfEdge::~HDS_HalfEdge()
{

}