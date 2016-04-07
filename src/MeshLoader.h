#ifdef _MSC_VER
#pragma once
#pragma warning (disable:4996)
#endif // _MSVC

#include "common.h"

struct PolyIndex
{
	uint32_t size;
	ui32s_t v;
	ui32s_t uv;
	ui32s_t n;
	void push_back(uint32_t* ids)
	{
		v.push_back(ids[0] - 1);
		if (ids[1] > 0)
		{
			uv.push_back(ids[1] - 1);
		}
		if (ids[2] > 0)
		{
			uv.push_back(ids[2] - 1);
		}
	}
};
class MeshLoader
{
public:
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

	void release();
	void load_from_string(const char* filename);
	void load_from_file(const char* filename);

	void exportVBO(floats_t* verts = nullptr,
		floats_t* texcoords = nullptr,
		floats_t* norms = nullptr, ui32s_t* fids = nullptr) const;
private:
	char* readfile(const char* filename);
	index_t facetype(const char* str, uint32_t* val);

	friend class HDS_Mesh;
private:
	floats_t vertices;
	floats_t uvs;
	floats_t normals;
	vector<PolyIndex*> polys;
};