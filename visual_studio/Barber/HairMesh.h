#pragma once
#include "common.h"
#include "hds_mesh.h"
#include <QQuaternion>
#include <QMatrix4x4>
#include <QColor>

class LayeredHairMesh
{
public:
	LayeredHairMesh(const HDS_Face* src, bool deepSet = true);
	~LayeredHairMesh();
	void extrude(double val);
	void translate(int lv, const QVector3D &dir, int mode = 0);
	void scale(int lv, double xval, double yval, double zval);
	void rotate(int lv, double angle, int mode = 0);
	void remove(int lv);

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
	QColor color;
};

class HairMesh
{
public:
	HairMesh(const HDS_Mesh* src, const char *filename = nullptr);
	~HairMesh();
	void push_back(LayeredHairMesh* layer);
	int sizeAtStroke(int i) const;
	bool empty() const;
	void save(const char* filename) const;
	void exportIndexedVBO(
		vector<float>* vtx_array = nullptr,
		vector<uint>* idx_array = nullptr,
		vector<uint>* vtx_offset_array = nullptr,
		vector<uint>* idx_offset_array = nullptr,
		vector<float>* layer_color = nullptr) const;
private:
	friend class OGLViewer;
private:
	const HDS_Mesh* ref_mesh;
	vector<LayeredHairMesh*> layers;

};

