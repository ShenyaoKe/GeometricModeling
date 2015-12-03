#include "Simplification.h"

Simplification::Simplification(uint lv, const mesh_t* src)
	: origin_mesh(src)
{
	const mesh_t* curMesh = origin_mesh;
	for (int i = 0; i < lv; i++)
	{
		mesh_t* simp_ret = simplify(curMesh);
		cout << "Simplification at level " << i + 1 << "\n\t";
		simp_mesh.push_back(simp_ret);
		curMesh = simp_ret;
	}
}

Simplification::~Simplification()
{
}

void Simplification::exportIndexedVBO(int lv,
	vector<float>* vtx_array,
	vector<float>* uv_array,
	vector<float>* norm_array,
	vector<ushort>* idx_array)
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

void Simplification::exportVBO(int lv,
	vector<float>* vtx_array,
	vector<float>* uv_array,
	vector<float>* norm_array)
{
	if (lv < 1 || simp_mesh.size() == 0)// 0 or negative value
	{
		origin_mesh->exportVBO(vtx_array, uv_array, norm_array);
	}
	else // 1 or bigger
	{
		lv = min(lv - 1, (int)simp_mesh.size() - 1);
		simp_mesh[lv]->exportVBO(vtx_array, uv_array, norm_array);
	}
}

mesh_t* Simplification::simplify(const mesh_t* src)
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
	cout << "Current face number: " << ret->faceSet.size() << endl;
	clock_t simp_start, start_time, end_time;//Timer
	simp_start = start_time = clock();
#endif
	//unordered_map<int, vector<vert_t*>*> neighborVertMap;
	unordered_set<he_t*> unsafeEdges;
	unordered_set<he_t*> dirtyEdges;
	unordered_set<he_t*> invalidEdges;
	unordered_set<face_t*> invalidFaces;
	vector<vert_t*> invalidVert;
	//unordered_map<int, QEF*> vertQEFs;
	//QEF* vertQEFs = new QEF[src->vertSet.size()];
	vector<QEF*> vertQEFs(ret->vertSet.size(), nullptr);
	//unordered_map<int, QEF*> heQEFs;
	vector<QEF*> heQEFs(ret->heSet.size(), nullptr);
	//priority_queue<QEF*, vector<QEF*>, QEF> qefPrQueue;

	for (auto vert : ret->vertSet)
	{
		// For all vert, find neighbors
		auto neighborVerts = vert->neighbors();
		// If neighbor < 4, mark as unsafe edges
		if (neighborVerts.size() < 4)
		{
			auto curHE = vert->he;
			do 
			{
				unsafeEdges.insert(curHE->next);
				curHE = curHE->flip->next;
			} while (curHE != vert->he);
		}
		// Calculate QEF
		QEF* qef = new QEF;
		qef->num = neighborVerts.size();
		for (auto nv : neighborVerts)
		{
			qef->vSum += nv->pos;
			qef->vMulSum += nv->pos.lengthSquared();
		}
		qef->vert = vert;
		vertQEFs[vert->index] = qef;
		//vertQEFs.insert(make_pair(vert->index, qef));
	}
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tVetices QEF Time:\t" << (float)(end_time - start_time) / CLOCKS_PER_SEC << "s\n";
	start_time = clock();
#endif
	// For all edges not in unsafe edges, compute combined QEF
	vector<bool> visitedHE(ret->heSet.size(), false);
	vector<QEF*> heQEFVector;
	for (auto he : ret->heSet)
	{
		auto hef = he->flip;

		// Skip unsafe edges
		if (unsafeEdges.find(he) != unsafeEdges.end() ||
			unsafeEdges.find(hef) != unsafeEdges.end())
		{
			continue;
		}
		// Unvisited edges
		if (!visitedHE[he->index] && !visitedHE[hef->index])
		{
			visitedHE[he->index] = true;
			visitedHE[hef->index] = true;
			//visitedHE.insert(he);
			//visitedHE.insert(hef);

			auto vQEF0 = vertQEFs[he->v->index];
			auto vQEF1 = vertQEFs[hef->v->index];
			QEF* heQEF = new QEF;
			*heQEF = *vQEF0 + *vQEF1;
			heQEF->he = he;
			//heQEFs.insert(make_pair(he->index, heQEF));
			heQEFs[he->index] = heQEF;

			heQEF->calMinError();
			heQEFVector.push_back(heQEF);
			/*qefPrQueue.push(heQEF);*/
		}
	}
	priority_queue<QEF*, vector<QEF*>, QEF> qefPrQueue(heQEFVector.begin(), heQEFVector.end());
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tEdges QEF Time:\t" << (float)(end_time - start_time) / CLOCKS_PER_SEC << "s\n";
	start_time = clock();
#endif
	// Collapse edges
	int polyCount = ret->faceSet.size();
	int targPolyCount = polyCount * percent;
	while (!qefPrQueue.empty() && polyCount > targPolyCount)
	{
		//cout << "QEF:\t" << qefPrQueue.top()->err << endl;
		auto qef = qefPrQueue.top();
		auto curHE = qef->he;

		if (/*unsafeEdges.find(curHE) == unsafeEdges.end()
			&& */invalidEdges.find(curHE) == invalidEdges.end()
			&& dirtyEdges.find(curHE) == dirtyEdges.end()
			&& dirtyEdges.find(curHE->flip) == dirtyEdges.end())
		{
			// collapse edge
			// record invalid edges and dirty edges
			invalidVert.push_back(curHE->flip->v);
			ret->collapse(curHE, qef->minErrPos,
				&dirtyEdges, &invalidEdges, &invalidFaces);
			polyCount -= 2;
		}
		
		qefPrQueue.pop();
	}
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tCollapsing Time:\t" << (float)(end_time - start_time) / CLOCKS_PER_SEC << "s\n";
	start_time = clock();
#endif
	for (auto vert : invalidVert)
	{
		ret->vertSet.erase(vert);
		ret->vertMap.erase(vert->index);
		delete vert;
		vert = nullptr;
	}
	for (auto he : invalidEdges)
	{
		ret->heSet.erase(he);
		ret->heMap.erase(he->index);
		delete he;
		he = nullptr;
	}
	for (auto he : ret->heSet)
	{
		ret->validateEdge(he);
	}
	for (auto face : invalidFaces)
	{
		ret->faceSet.erase(face);
		ret->faceMap.erase(face->index);
		delete face;
	}
	ret->reIndexVert();
	ret->reIndexFace();
	for (auto vQEF : vertQEFs)
	{
		delete vQEF;
	}
	vertQEFs.clear();
	for (auto heQEF : heQEFs)
	{
		delete heQEF;
	}
	//ret->validate();
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tSimplification Time:\t" << end_time - simp_start << "ms\n";
	start_time = clock();
	cout << "Simplified face number: " << ret->faceSet.size() << endl;
#endif

	return ret;
}

int Simplification::getLevel() const
{
	return simp_mesh.size();
}

