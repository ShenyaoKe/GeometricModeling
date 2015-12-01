#pragma once

#ifndef __SIMPLIFICATION__
#define __SIMPLIFICATION__

#include "hds_mesh.h"
typedef QVector3D point_t;
typedef HDS_Face face_t;
typedef HDS_Vertex vert_t;
typedef HDS_HalfEdge he_t;
typedef HDS_Mesh mesh_t;

struct QEF
{
	int num;		//n
	QVector3D vSum;	// sum(vi)
	double vMulSum;	// sum(viT * vi)
	double err;
	union src
	{
		vert_t* vert;
		he_t* he;
	};

	QEF(int n, const QVector3D &vs, double vms)
		: num(n), vSum(vs), vMulSum(vms), err(0){}
	double calError()
	{
		return;
	}
	bool operator < (const QEF &cmp)
	{
		return error < cmp.error;
	}
	QEF operator + (const QEF &other)
	{
		return QEF(count + othercount, vSum + othervSum, vMulSum + othervMulSum);
	}
};

class Simplification
{
public:
	Simplification(uint lv, const mesh_t* src);
	~Simplification();

	void simplify(const mesh_t* src = nullptr);

	void exportIndexedVBO(int lv = 0,
		vector<float>* vtx_array = nullptr,
		vector<float>* uv_array = nullptr,
		vector<float>* norm_array = nullptr,
	
		vector<ushort>* idx_array = nullptr);
private:
private:
	const mesh_t* origin_mesh;
	vector<mesh_t*> simp_mesh;
};


#endif // __SIMPLIFICATION__