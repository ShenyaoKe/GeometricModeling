#include "Simplification.h"

Simplification::Simplification(uint lv, const mesh_t* src)
	: origin_mesh(src)
{
	const mesh_t* curMesh = origin_mesh;
	for (int i = 0; i < lv; i++)
	{
		mesh_t* simp_ret;// = simplify(curMesh);
		cout << "Subdivided at level " << i + 1 << "\n\t";
		simp_mesh.push_back(simp_ret);
		curMesh = simp_ret;
	}
}

Simplification::~Simplification()
{
}

void Simplification::exportIndexedVBO(int lv, vector<float>* vtx_array, vector<float>* uv_array, vector<float>* norm_array, vector<ushort>* idx_array)
{
	if (lv < 1 || simp_mesh.size() == 0)// 0 or negative value
	{
		origin_mesh->exportIndexedVBO(vtx_array, uv_array, norm_array, idx_array);
	}
	else // 1 or bigger
	{
		lv = min(lv - 1, (int)simp_mesh.size() - 1);
		simp_mesh[lv]->exportIndexedVBO(vtx_array, uv_array, norm_array, idx_array);
	}
}

void Simplification::simplify(const mesh_t* src)
{
	// Initialize simplification by getting the latest mesh
	//const mesh_t* src;
	if (src == nullptr || simp_mesh.size() == 0)
	{
		src = origin_mesh;
	}
	else
	{
		src = simp_mesh.back();
	}

	mesh_t* ret = new mesh_t(*src);

#ifdef _DEBUG
	clock_t subdiv_start, start_time, end_time;//Timer
	subdiv_start = start_time = clock();
#endif







#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tGenerate Face Center:\t" << end_time - start_time << "ms\n";
	start_time = clock();
#endif
}
