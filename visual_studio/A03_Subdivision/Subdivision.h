#pragma once
#ifndef __SUBDIVISION__
#define __SUBDIVISION__

#include "hds_mesh.h"
typedef QVector3D point_t;
typedef HDS_Face face_t;
typedef HDS_Vertex vert_t;
typedef HDS_HalfEdge he_t;
typedef HDS_Mesh mesh_t;

/************************************************************************/
/* Level 0: Original Mesh                                               */
/* Level i: Subdivision mesh at level i, i.e. subd_mesh[i - 1]			*/
/************************************************************************/
class Subdivision
{
public:

	Subdivision(uint lv, const mesh_t* origin);
	//Subdivision(const Subdivision* subd_mesh);
	~Subdivision();

	void subdivide();
	uint getLevel() const;
	void saveAsOBJ(uint lv, const char* filename) const;
	void exportIndexedVBO(int lv = 0,
		vector<float>* vtx_array = nullptr,
		vector<float>* uv_array = nullptr,
		vector<float>* norm_array = nullptr,
		vector<uint>* idx_array = nullptr);
private:
	mesh_t* subdivide(const mesh_t* origin);
private:
	//uint level;
	const mesh_t* origin_mesh;
	vector<mesh_t*> subd_mesh;
};


#endif // __SUBDIVISION__