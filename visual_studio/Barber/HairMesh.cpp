#include "hairMesh.h"


LayeredHairMesh::LayeredHairMesh(const HDS_Face* src, bool deepSet)
	: root(src)
	, vertice(src->corners())
	//, color(rand() % 256, rand() % 256, rand() % 256)
{
	seg = vertice.size();
	if (deepSet)
	{
		for (auto v : vertice)
		{
			points.push_back(v->pos);
		}
		color.setHsv(rand() % 360, 180, 255);
	}
	
}

LayeredHairMesh::~LayeredHairMesh()
{
	root = nullptr;
	vertice.clear();
	points.clear();
}

HairMesh::HairMesh(const HDS_Mesh* src, const char *filename)
	: ref_mesh(src)
{
	if (filename == nullptr)
	{
		return;
	}
	// Load from file
	FILE *HMS_File;
	errno_t err = fopen_s(&HMS_File, filename, "r");
	if (err)
	{
		printf("Can't read file %s!\n", filename);
		return;
	}
	float x, y, z;
	int fid;
	LayeredHairMesh* curLayer = nullptr;
	while (true)
	{
		char lineHeader[128];
		int res = fscanf_s(HMS_File, "%s", &lineHeader, _countof(lineHeader));
		if (res == EOF)
		{
			break;
		}
		if (strcmp(lineHeader, "L") == 0)
		{
			fscanf_s(HMS_File, " F%d\n", &fid);
			LayeredHairMesh* layer = new LayeredHairMesh(ref_mesh->faceMap.at(fid), false);
			curLayer = layer;
			layers.push_back(layer);
		}
		else if (strcmp(lineHeader, "c") == 0)
		{
			char color_str[8];
			fscanf_s(HMS_File, " %s\n", &color_str, _countof(color_str));
			curLayer->color = QColor(color_str);
		}
		else if (strcmp(lineHeader, "v") == 0)
		{

			fscanf_s(HMS_File, " %f %f %f\n", &x, &y, &z);
			curLayer->points.push_back(QVector3D(x, y, z));
		}
		else//if (strcmp(lineHeader, "#") == 0)
		{
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, HMS_File);
		}
	}
	fclose(HMS_File);
}

HairMesh::~HairMesh()
{
	for (auto layer : layers)
	{
		delete layer;
	}
	layers.clear();
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

bool HairMesh::empty() const
{
	return layers.size() == 0;
}

void HairMesh::save(const char* filename) const
{
	FILE *HMS_File;
	errno_t err = fopen_s(&HMS_File, filename, "w");
	if (err)
	{
		printf("Can't write to file %s!\n", filename);
		return;
	}
	fprintf(HMS_File, "#Hair Mesh\n");
	for (int i = 0; i < layers.size(); i++)
	{
		auto curLayer = layers[i];
		// Write stroke ref face id
		fprintf(HMS_File, "L F%d\n", curLayer->root->index);

		//Write stroke colors
		fprintf(HMS_File, "c %s\n", curLayer->color.name().toStdString().c_str());

		for (int i = 0; i < curLayer->points.size(); i++)
		{
			auto& curPt = curLayer->points[i];
			fprintf(HMS_File, "v %f %f %f\n", curPt.x(), curPt.y(), curPt.z());
		}
	}

	fclose(HMS_File);
}

void HairMesh::exportIndexedVBO(
	vector<float>* vtx_array, vector<uint>* idx_array,
	vector<uint>* vtx_offset_array, vector<uint>* idx_offset_array,
	vector<float>* layer_color) const
{
	int layerSize = layers.size();
	if (vtx_array != nullptr)
	{
		vtx_array->clear();
		if (vtx_offset_array != nullptr)
		{
			vtx_offset_array->clear();
			vtx_offset_array->reserve(layerSize + 1);
		}
		if (idx_offset_array)
		{
			idx_offset_array->clear();
			idx_offset_array->reserve(layerSize + 1);
		}
		int size = 0;
		for (auto layer : layers)
		{
			vtx_offset_array->push_back(size);
			size += layer->points.size();
		}
		vtx_offset_array->push_back(size);
		vtx_array->reserve(size * 3);
	}
	if (idx_array != nullptr)
	{
		idx_array->clear();
	}
	
	idx_offset_array->push_back(0);
	for (int i = 0; i < layerSize; i++)
	{
		auto layer = layers[i];
		layer->exportIndexedVBO((*vtx_offset_array)[i], vtx_array, idx_array);
		idx_offset_array->push_back(idx_array->size());
	}
	if (layer_color != nullptr)
	{
		layer_color->clear();
		layer_color->reserve(layerSize * 3);
		for (int i = 0; i < layerSize; i++)
		{
			auto& curColor = layers[i]->color;
			layer_color->push_back(static_cast<float>(curColor.redF()));
			layer_color->push_back(static_cast<float>(curColor.greenF()));
			layer_color->push_back(static_cast<float>(curColor.blueF()));
		}
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

void LayeredHairMesh::translate(int lv, const QVector3D &dir, int mode)
{
	if (lv < 1)
	{
		return;
	}
	lv = min(lv, (int)points.size() / seg - 1);
	lv *= seg;

	for (int i = lv; i < lv + seg; i++)
	{
		points[i] += dir;
	}
}

void LayeredHairMesh::scale(int lv, double xval, double yval, double zval)
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
	QVector3D dx = (points[lv + 1] - points[lv]).normalized();
	QVector3D dy = QVector3D::crossProduct(normal, dx);

	for (int i = lv; i < lv + seg; i++)
	{
		points[i] = QVector3D::dotProduct(points[i] - center, dx) * yval * dx
			+ QVector3D::dotProduct(points[i] - center, dy) * xval * dy
			+ QVector3D::dotProduct(points[i] - center, normal) * xval * normal
			+ center;
	}
}

void LayeredHairMesh::rotate(int lv, double val, int mode)
{
	if (lv < 1)
	{
		return;
	}
	lv = min(lv, (int)points.size() / seg - 1);
	lv *= seg;

	QMatrix4x4 rot;
	QVector3D center;
	for (int i = lv; i < lv + seg; i++)
	{
		center += points[i];
	}
	center /= seg;
	QVector3D normal = QVector3D::crossProduct(
		points[lv] - center, points[lv + 1] - center).normalized();
	QVector3D dx = (points[lv + 1] - points[lv]).normalized();
	QVector3D dy = QVector3D::crossProduct(normal, dx);
	switch (mode)
	{
	case 0:// normal
	{

		rot.rotate(val, normal);
		break;
	}
	case 1:// Y dir
	{
		rot.rotate(val, dy);
		break;
	}
	case 2:// Y dir
	{
		rot.rotate(val, dx);
		break;
	}
	default:
		break;
	}
	
	for (int i = lv; i < lv + seg; i++)
	{
		points[i] = rot * (points[i] - center) + center;
	}
}

void LayeredHairMesh::remove(int lv)
{
	auto curPt = points.begin() + lv * seg;
	if (lv > 0)
	{

		points.erase(curPt, curPt + seg);
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
