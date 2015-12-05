#include "hairMesh.h"


HairMeshLayer::HairMeshLayer(const HDS_Face* src)
	: root(src)
	, vertice(src->corners())
{
	seg = vertice.size();
	for (auto v : vertice)
	{
		points.push_back(v->pos);
	}
}

HairMesh::HairMesh()
{

}

HairMesh::~HairMesh()
{
}

void HairMesh::push_back(HairMeshLayer* layer)
{
	layers.push_back(layer);
}

void HairMeshLayer::exportIndexedVBO(
	vector<float>* vtx_array, vector<uint>* idx_array) const
{
	bool has_vert(false), has_texcoord(false), has_normal(false), has_uid(false);

	
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

	// If no mesh extruded
	if (points.size() == seg)
	{
		for (int i = 0; i < seg; i++)
		{
			idx_array->push_back(i);
		}
		return;
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
