#pragma once
#include "common.h"
#include "hds_mesh.h"
#include "Accel/KdTreeAccel.h"
#include <QQuaternion>
#include <QMatrix4x4>
#include <QColor>

const double timestep = 1.0 / 30.0;
const QVector3D gravity(0, -6.2, 0);
const QVector3D windforce(0, 0, -4);
const QVector3D sphere(0, 6.2, -1.9);
const double radius = 16.5;
const double radSq = radius * radius;
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
		vector<uint>* idx_array = nullptr,
		vector<float>* sim_vtx_array = nullptr) const;

private:
	void externalUpdate(vector<QVector3D> &state);
	void internalUpdate(vector<QVector3D> &state);
	vector<QVector3D> internalForces(const vector<QVector3D> &state) const;
	void collisionDetect(vector<QVector3D> &state);
	void simulate();
private:
	friend class HairMesh;
	friend class OGLViewer;
private:
	const HDS_Face* root;
	int seg;// Number of points on each layer
	vector<HDS_Vertex*> vertice;
	vector<QVector3D> points;
	QColor color;
private:
	const KdTreeAccel* acceltree;
	vector<QVector3D> simPts;
	vector<QVector3D> simVel;
	vector<float> restLen;
	vector<float> restAng;
};
inline vector<QVector3D> operator + (const vector<QVector3D> &lhs, const vector<QVector3D> &rhs)
{
	vector<QVector3D> ret(lhs.size());
	if (lhs.size() != rhs.size())
	{
		return ret;
	}
	for (int i = 0; i < lhs.size(); i++)
	{
		ret[i] = lhs[i] + rhs[i];
	}
	return ret;
}
inline vector<QVector3D> operator * (const vector<QVector3D> &vec, double scalor)
{
	vector<QVector3D> ret(vec.size());
	for (int i = 0; i < vec.size(); i++)
	{
		ret[i] = vec[i] * scalor;
	}
	return ret;
}
class HairMesh
{
public:
	HairMesh(const HDS_Mesh* src, const char *filename = nullptr);
	~HairMesh();
	void push_back(LayeredHairMesh* layer);
	int sizeAtStroke(int i) const;
	bool empty() const;
	void save(const char* filename) const;

	// Simulation
	void initialSimulation();
	void simulate();
	void calcNewVel();
	void assignCollision(const KdTreeAccel* kdtree);
	// Rendering
	void exportIndexedVBO(
		vector<float>* vtx_array = nullptr,
		vector<uint>* idx_array = nullptr,
		vector<uint>* vtx_offset_array = nullptr,
		vector<uint>* idx_offset_array = nullptr,
		vector<float>* layer_color = nullptr,
		vector<float>* sim_vtx_array = nullptr) const;
private:
	friend class OGLViewer;
private:
	const KdTreeAccel* acceltree;
	const HDS_Mesh* ref_mesh;
	vector<LayeredHairMesh*> layers;

};

