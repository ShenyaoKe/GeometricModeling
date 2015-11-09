#include "hds_vertex.h"
#include "hds_halfedge.h"


size_t HDS_Vertex::uid = 0;

HDS_Vertex::HDS_Vertex()
	: index(-1)
	, pos(0, 0, 0), normal(0, 1, 0)
	, he(nullptr)
{
}

HDS_Vertex::HDS_Vertex(const HDS_Vertex& v)
	: index(v.index)
	, pos(v.pos), normal(v.normal)
	, he(nullptr)
{
}

HDS_Vertex::HDS_Vertex(const QVector3D &p)
	: index(-1)
	, pos(p), normal(0, 1, 0)
	, he(nullptr)
{
}

HDS_Vertex::~HDS_Vertex()
{
}

vector<HDS_Vertex*> HDS_Vertex::neighbors() const
{
	vector<HDS_Vertex*> ret;

	HDS_HalfEdge *curHE = he;
	do
	{
		ret.push_back(curHE->flip->v);
		curHE = curHE->flip->next;
	} while( curHE != he );

	return ret;
}
void HDS_Vertex::computeNormal()
{
	auto prevHE = he;
	auto curHE = he->flip->next;
	normal = QVector3D(0, 0, 0);
	do
	{
		/*
		if (!prevHE->f->isCutFace)
		{
			QVector3D v1 = prevHE->flip->v->pos - prevHE->v->pos;
			QVector3D v2 = curHE->flip->v->pos - curHE->v->pos;
			normal += QVector3D::crossProduct(v2, v1) * acos(QVector3D::dotProduct(v2.normalized(), v1.normalized()));
		}*/
		prevHE = curHE;
		curHE = prevHE->flip->next;
	} while (prevHE != he);

	normal.normalize();
}
