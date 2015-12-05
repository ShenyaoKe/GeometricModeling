#pragma once
#include "common.h"
#include "hds_mesh.h"
#include <QQuaternion>
#include <QMatrix4x4>

class LayeredHairMesh
{
public:
	LayeredHairMesh(const HDS_Face* src);
	~LayeredHairMesh();
	void extrude(double val);
	void shrink(int lv, double scale);
	void twist(int lv, double angle);
	void exportIndexedVBO(int offset = 0,
		vector<float>* vtx_array = nullptr,
		vector<uint>* idx_array = nullptr) const;
private:
	friend class HairMesh;
	friend class OGLViewer;
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
	void push_back(LayeredHairMesh* layer);
	int sizeAtStroke(int i) const;
	void exportIndexedVBO(
		vector<float>* vtx_array = nullptr,
		vector<uint>* idx_array = nullptr,
		vector<uint>* offset_array = nullptr,
		vector<float>* layer_color = nullptr) const;
private:
	friend class OGLViewer;
private:
	const HDS_Mesh* ref_mesh;
	vector<LayeredHairMesh*> layers;

};

