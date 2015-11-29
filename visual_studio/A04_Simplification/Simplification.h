#pragma once

#ifndef __SIMPLIFICATION__
#define __SIMPLIFICATION__

#include "hds_mesh.h"
typedef QVector3D point_t;
typedef HDS_Face face_t;
typedef HDS_Vertex vert_t;
typedef HDS_HalfEdge he_t;
typedef HDS_Mesh mesh_t;

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