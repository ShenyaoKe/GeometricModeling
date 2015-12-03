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
	QVector3D minErrPos;
	union
	{
		vert_t* vert;
		he_t* he;
	};

	QEF() : num(0), vSum(0, 0, 0), vMulSum(0), err(0) {}
	QEF(int n, const QVector3D &vs, double vms)
		: num(n), vSum(vs), vMulSum(vms), err(0){}
	~QEF(){}
	double calMinError()
	{
		minErrPos = vSum / static_cast<float>(num);
		err = minErrPos.lengthSquared() * num + 2 * QVector3D::dotProduct(minErrPos, vSum) + vMulSum;

		return err;
	}
	QEF operator + (const QEF &other)
	{
		return QEF(num + other.num, vSum + other.vSum, vMulSum + other.vMulSum);
	}

	/*bool operator() (const QEF &lhs, const QEF &rhs) const
	{
		return lhs.err > rhs.err;
	}*/
	bool operator() (const QEF *lhs, const QEF *rhs) const
	{
		return lhs->err > rhs->err;
	}
};
/*
bool operator < (const QEF &lhs, const QEF &rhs)
{
	return lhs.err < rhs.err;
}*/

class Simplification
{
public:
	Simplification(uint lv, const mesh_t* src);
	~Simplification();

	void simplify();
	int getLevel() const;
	void saveAsOBJ(uint lv, const char* filename) const;

	void exportVBO(int lv = 0,
		vector<float>* vtx_array = nullptr,
		vector<float>* uv_array = nullptr,
		vector<float>* norm_array = nullptr) const;
	void exportIndexedVBO(int lv = 0,
		vector<float>* vtx_array = nullptr,
		vector<float>* uv_array = nullptr,
		vector<float>* norm_array = nullptr,
		vector<uint>* idx_array = nullptr) const;
private:
	mesh_t* simplify(const mesh_t* src);
private:
	friend class OGLViewer;
	const mesh_t* origin_mesh;
	vector<mesh_t*> simp_mesh;

	double percent = 0.5;
};


#endif // __SIMPLIFICATION__