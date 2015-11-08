#pragma once

#include "common.h"
#include <QVector2D>
#include <QVector3D>

struct FaceIndex
{
	vector<int> vtx;	//vertex
	vector<int> uv;	//texture coordinate
	vector<int> n;	//normal
	void printInfo() const
	{
		cout << "Quad face index: ";
		for (int i = 0; i < vtx.size(); i++)
		{
			cout << vtx[i] << "/" << uv[i] << "/" << n[i] << "\t";
		}
		cout << endl;
	}
};
struct QuadFaceIndex
{
	int vtx[4];	//vertex
	int uv[4];	//texture coordinate
	int n[4];	//normal
	void printInfo() const
	{
		cout << "Quad face index: ";
		for (int i = 0; i < 4; i++)
		{
			cout << vtx[i] << "/" << uv[i] << "/" << n[i] << "\t";
		}
		cout << endl;
	}
};
class MeshLoader
{
public:
	MeshLoader(const string &filename);
	~MeshLoader();
private:
	friend class HDS_Mesh;
private:
	vector<QVector3D> vertices;
	vector<QVector2D> uvs;
	vector<QVector3D> normals;
	vector<QuadFaceIndex> fids;
};

