#include "hairMesh.h"


LayeredHairMesh::LayeredHairMesh(const HDS_Face* src)
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

void HairMesh::push_back(LayeredHairMesh* layer)
{
	layers.push_back(layer);
}

int HairMesh::sizeAtStroke(int i) const
{
	auto curLayer = layers[i];
	return curLayer->points.size() / curLayer->seg;
}

void HairMesh::exportIndexedVBO(
	vector<float>* vtx_array, vector<uint>* idx_array,
	vector<uint>* offset_array, vector<float>* layer_color) const
{
	
	if (vtx_array != nullptr)
	{
		vtx_array->clear();
		if (offset_array != nullptr)
		{
			offset_array->clear();
			offset_array->reserve(layers.size());
		}
		int size = 0;
		for (auto layer : layers)
		{
			offset_array->push_back(size);
			size += layer->points.size();
		}
		vtx_array->reserve(size * 3);
	}
	if (idx_array != nullptr)
	{
		idx_array->clear();
	}
	for (int i = 0; i < layers.size(); i++)
	{
		auto layer = layers[i];
		layer->exportIndexedVBO((*offset_array)[i], vtx_array, idx_array);
	}
}

void LayeredHairMesh::extrude(double val)
{
	int curSize = points.size();
	int v0 = curSize - seg;
	int v1 = v0 + 1;
	int v2 = v1 + 1;
	int v3 = v2 + 1;
	QVector3D normal = val * QVector3D::crossProduct(
		points[v2] - points[v0], points[v3] - points[v1]).normalized();
	for (int i = curSize - seg; i < curSize; i++)
	{
		points.push_back(points[i] + normal);
	}
}

void LayeredHairMesh::shrink(int lv, double scale)
{
	if (lv < 1)
	{
		return;
	}
	lv = min(lv, (int)points.size() / seg - 1);
	lv *= seg;

	QVector3D center;
	for (int i = lv; i < lv + seg; i++)
	{
		center += points[i];
	}
	center /= seg;
		
	for (int i = lv; i < lv + seg; i++)
	{
		points[i] = (points[i] - center) * scale + center;
	}
}

void LayeredHairMesh::twist(int lv, double angle)
{
	if (lv < 1)
	{
		return;
	}
	lv = min(lv, (int)points.size() / seg - 1);
	lv *= seg;

	QVector3D center;
	for (int i = lv; i < lv + seg; i++)
	{
		center += points[i];
	}
	center /= seg;
	QVector3D normal = QVector3D::crossProduct(
		points[lv] - center, points[lv + 1] - center).normalized();
	QMatrix4x4 rot;
	rot.rotate(angle, normal);

	for (int i = lv; i < lv + seg; i++)
	{
		points[i] = rot * (points[i] - center) + center;
	}
}

void LayeredHairMesh::exportIndexedVBO(int offset,
	vector<float>* vtx_array, vector<uint>* idx_array) const
{
	bool has_vert(false), has_texcoord(false), has_normal(false), has_uid(false);

	
	/*if (vtx_array != nullptr)
	{
		vtx_array->clear();
		vtx_array->reserve(points.size() * 3);
		has_vert = true;
	}
	if (idx_array != nullptr)
	{
		idx_array->clear();
	}*/
	
	for (auto pt : points)
	{
		vtx_array->push_back(pt.x());
		vtx_array->push_back(pt.y());
		vtx_array->push_back(pt.z());
	}

	// If no mesh extruded
	for (int i = seg + offset; i < points.size() + offset; i++)
	{
		if ((i - offset) % seg == 0)
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

	for (int i = points.size() - seg + offset; i < points.size() + offset; i++)
	{
		idx_array->push_back(i);
	}
}
