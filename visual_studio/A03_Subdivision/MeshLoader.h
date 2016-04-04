#pragma once

#include "common.h"
#include <QVector2D>
#include <QVector3D>

struct FaceIndex
{
	vector<int32_t> v;
	vector<int32_t> uv;
	vector<int32_t> n;
	void push_back(int32_t* ids)
	{
		v.push_back(ids[0]);
		if (ids[1] > 0)
		{
			uv.push_back(ids[1]);
		}
		if (ids[2] > 0)
		{
			uv.push_back(ids[2]);
		}
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

