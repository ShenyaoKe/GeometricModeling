#pragma once
#include "common.h"
#include "hds_mesh.h"
class HairMeshLayer
{
public:
	HairMeshLayer(const HDS_Face* src);
	~HairMeshLayer();

	void exportIndexedVBO(vector<float>* vtx_array = nullptr,
		vector<uint>* idx_array = nullptr) const;
private:
	const HDS_Face* root;
	int seg;// Number of points on each layer
	vector<HDS_Vertex*> vertice;
	vector<QVector3D> points;
};

class HairMesh
{
public:
	HairMesh();
	~HairMesh();
	void push_back(HairMeshLayer* layer);
	void exportIndexedVBO() const;
private:
	const HDS_Mesh* ref_mesh;
	vector<HairMeshLayer*> layers;
};

