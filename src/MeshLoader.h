#pragma once
#ifdef _MSC_VER
#pragma warning (disable:4996)
#endif // _MSVC

#include "common.h"
#include <QVector2D>
#include <QVector3D>

struct PolyIndex
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

struct FaceIndex
{
	vector<int> vtx;	//vertex
	vector<int> uv;	//texture coordinate
	vector<int> n;	//normal
	FaceIndex(int const* indices , int len)
	{
		vtx.reserve(len);
		uv.reserve(len);
		n.reserve(len);
		for (int i = 0; i < len; i++)
		{
			vtx.push_back(*indices++);
			uv.push_back(*indices++);
			n.push_back(*indices++);
		}
	}
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
	int vtx[4] = {};	//vertex
	int uv[4] = {};	//texture coordinate
	int n[4] = {};	//normal
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
	//MeshLoader(const string &filename);
	MeshLoader(const char* filename);
	~MeshLoader();
	
	enum index_t : uint8_t
	{
		V = 1 << 0,
		UV = 1 << 1,
		NORM = 1 << 2,
		VT = V | UV,
		VN = V | NORM,
		VTN = V | UV | NORM
	};
private:
	

	char* readfile(const char* filename);
	index_t facetype(const char* str, int* val);

	friend class HDS_Mesh;
private:
	vector<QVector3D> vertices;
	vector<QVector2D> uvs;
	vector<QVector3D> normals;
	//vector<QuadFaceIndex> fids;
	//vector<FaceIndex> fids;
	vector<PolyIndex*> polys;
};

