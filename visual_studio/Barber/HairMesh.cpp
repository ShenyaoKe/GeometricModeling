#include "hairMesh.h"


HairMesh::HairMesh()
{
}


HairMesh::~HairMesh()
{
}

void HairMeshLayer::exportIndexedVBO(
	vector<float>* vtx_array, vector<uint>* idx_array) const
{
	bool has_vert(false), has_texcoord(false), has_normal(false), has_uid(false);

	if (points.size() < seg)
	{
		//export root
		return;
	}
	if (vtx_array == nullptr)
	{
		vtx_array->clear();
		vtx_array->reserve(points.size() * 3);
		has_vert = true;
	}
	
	for (auto pt : points)
	{
		vtx_array->push_back(pt.x());
		vtx_array->push_back(pt.y());
		vtx_array->push_back(pt.z());
	}
	for (int i = seg; i < points.size() ; i++)
	{
		if (i % seg == 0)
		{
			idx_array->push_back(static_cast<uint>(i));
			idx_array->push_back(static_cast<uint>(i + seg - 1));
			idx_array->push_back(static_cast<uint>(i - 1));
			idx_array->push_back(static_cast<uint>(i - seg));
		}
		else
		{
			idx_array->push_back(static_cast<uint>(i));
			idx_array->push_back(static_cast<uint>(i - 1));
			idx_array->push_back(static_cast<uint>(i - 1 - seg));
			idx_array->push_back(static_cast<uint>(i - seg));
		}
		
	}
}
