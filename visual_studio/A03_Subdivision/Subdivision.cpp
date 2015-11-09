#include "Subdivision.h"



Subdivision::Subdivision(uint lv, const HDS_Mesh* origin)
	: level(lv)
	, origin_mesh(origin)
{
	const mesh_t* curMesh = origin_mesh;
	for (int i = 0; i < level; i++)
	{
		mesh_t* subd_ret = subdivide(curMesh);
		subd_mesh.push_back(subd_ret);
		curMesh = subd_ret;
	}
}

Subdivision::~Subdivision()
{
}

void Subdivision::exportIndexedVBO(int lv,
	vector<float>* vtx_array, vector<float>* uv_array,
	vector<float>* norm_array, vector<uint>* idx_array)
{
	if (lv < 1)// 0 or negative value
	{
		origin_mesh->exportIndexedVBO(vtx_array, uv_array, norm_array, idx_array);
	}
	else // 1 or bigger
	{
		subd_mesh[lv - 1]->exportIndexedVBO(vtx_array, uv_array, norm_array, idx_array);
	}
}

mesh_t* Subdivision::subdivide(const mesh_t* origin)
{
	mesh_t* ret = new mesh_t(*origin);

	unordered_map<face_t*, vert_t*> faceCenters;
	unordered_set<he_t*, vert_t*> edgeCenters;
	int index = origin->vertSet.size();
	for (auto face : ret->faceSet)
	{
		vert_t* fc = new vert_t(face->center());
		fc->index = HDS_Vertex::assignIndex();
		faceCenters.insert(make_pair(face, fc));
	}

	return ret;
}
