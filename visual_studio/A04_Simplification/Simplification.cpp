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
	for (auto mesh : simp_mesh)
	{
		mesh->releaseMesh();
		delete mesh;
	}
}

void Simplification::exportIndexedVBO(int lv,
	vector<float>* vtx_array,
	vector<float>* uv_array,
	vector<float>* norm_array,
	vector<uint>* idx_array) const
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
	vector<float>* norm_array) const
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
	vector<QEF*> vertQEF_vec(ret->vertSet.size(), nullptr);
	vector<QEF*> heQEF_vec(ret->heSet.size(), nullptr);

	auto vertQEF_Func = [&](vert_t* vert)
	{
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
		/*else
		{
			auto curHE = vert->he;
			do
			{
				unsafeEdges.erase(curHE->next);
				unsafeEdges.erase(curHE->next->flip);
				curHE = curHE->flip->next;
			} while (curHE != vert->he);
		}*/
		// Calculate QEF
		QEF* curVQEF = new QEF;
		curVQEF->num = neighborVerts.size();
		for (auto nv : neighborVerts)
		{
			curVQEF->vSum += nv->pos;
			curVQEF->vMulSum += nv->pos.lengthSquared();
		}
		curVQEF->vert = vert;
		vertQEF_vec[vert->index] = curVQEF;
	};
	for (auto vert : ret->vertSet)
	{
		vertQEF_Func(vert);
	}
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tVetices QEF Time:\t" << (float)(end_time - start_time) / CLOCKS_PER_SEC << "s\n";
	start_time = clock();
#endif
	// For all edges not in unsafe edges, compute combined QEF
	vector<bool> visitedHE(ret->heSet.size(), false);
	vector<QEF*> heQEF_InitList;

	auto heQEF_Func = [&](he_t* he)/*->QEF**/
	{
		auto hef = he->flip;

		// Skip unsafe edges
		if (unsafeEdges.find(he) != unsafeEdges.end() ||
			unsafeEdges.find(hef) != unsafeEdges.end())
		{
			return;
		}
		// Unvisited edges
		if (!visitedHE[he->index] && !visitedHE[hef->index])
		{
			visitedHE[he->index] = true;
			visitedHE[hef->index] = true;
			//visitedHE.insert(he);
			//visitedHE.insert(hef);

			auto vQEF0 = vertQEF_vec[he->v->index];
			auto vQEF1 = vertQEF_vec[hef->v->index];
			QEF* heQEF = new QEF;
			*heQEF = *vQEF0 + *vQEF1;
			heQEF->he = he;
			heQEF_vec[he->index] = heQEF;

			heQEF->calMinError();
			heQEF_InitList.push_back(heQEF);
			//return heQEF;
		}
	};
	for (auto he : ret->heSet)
	{
		/*auto curQEF = */heQEF_Func(he);
		/*if (curQEF != nullptr)
		{
			heQEF_InitList.push_back(curQEF);
		}*/
	}
	priority_queue<QEF*, vector<QEF*>, QEF> qefPrQueue(heQEF_InitList.begin(), heQEF_InitList.end());
	std::fill(visitedHE.begin(), visitedHE.end(), false);
	//unsafeEdges.clear();
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tEdges QEF Time:\t" << (float)(end_time - start_time) / CLOCKS_PER_SEC << "s\n";
	start_time = clock();
#endif
	// Collapse edges
	int polyCount = ret->faceSet.size();
	int targPolyCount = polyCount * percent;
	cout << "Current Polycount: " << polyCount << endl;
	cout << "Target: " << targPolyCount << endl;
	while (!qefPrQueue.empty() && polyCount > targPolyCount)
	{
		//cout << "QEF:\t" << qefPrQueue.top()->err << endl;
		auto qef = qefPrQueue.top();
		auto curHE = qef->he;

		if (invalidEdges.find(curHE) == invalidEdges.end())
		{
			if (dirtyEdges.find(curHE) == dirtyEdges.end()
				&& dirtyEdges.find(curHE->flip) == dirtyEdges.end())
			{
				// collapse edge
				// record invalid edges and dirty edges
				invalidVert.push_back(curHE->flip->v);
				ret->collapse(curHE, qef->minErrPos,
					&dirtyEdges, &invalidEdges, &invalidFaces);
				polyCount -= 2;
			}
			/*else// Dirty edge
			{
				qefPrQueue.pop();
				/ *delete qef;
				qef = nullptr;* /
				vertQEF_Func(curHE->v);
				vertQEF_Func(curHE->flip->v);
				heQEF_Func(curHE);
				qefPrQueue.push(heQEF_vec[curHE->index]);
				continue;
			}*/
			
		}
		
		qefPrQueue.pop();
	}
	cout << "Final Polycount: " << polyCount << endl;
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tCollapsing Time:\t" << (float)(end_time - start_time) / CLOCKS_PER_SEC << "s\n";
	start_time = clock();
#endif
	for (auto qef : heQEF_InitList)
	{
		delete qef;
		qef = nullptr;
	}
	
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
	ret->reIndexHE();
	ret->reIndexFace();
	for (auto vQEF : vertQEF_vec)
	{
		delete vQEF;
	}
	vertQEF_vec.clear();
	/*for (auto heQEF : heQEF_vec)
	{
		if (heQEF != nullptr)
		{
			delete heQEF;
		}
	}*/
	//ret->validate();
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tSimplification Time:\t" << end_time - simp_start << "ms\n";
	start_time = clock();
	cout << "Simplified face number: " << ret->faceSet.size() << endl;
#endif

	return ret;
}

void Simplification::simplify()
{
	if (simp_mesh.size() == 0)
	{
		simp_mesh.push_back(simplify(origin_mesh));
	} 
	else
	{
		simp_mesh.push_back(simplify(simp_mesh.back()));
	}
}

int Simplification::getLevel() const
{
	return simp_mesh.size();
}

void Simplification::saveAsOBJ(uint lv, const char* filename) const
{
	const mesh_t* curMesh = lv < 1 ? origin_mesh : simp_mesh[lv - 1];

	FILE *OBJ_File;
	errno_t err = fopen_s(&OBJ_File, filename, "w");
	if (err)
	{
		printf("Can't write to file %s!\n", filename);
		return;
	}
	fprintf(OBJ_File, "#Simplified Mesh Level %d\n", lv);

	for (int i = 0; i < curMesh->vertMap.size(); i++)
	{
		auto curVert = curMesh->vertMap.at(i);
		fprintf(OBJ_File, "v %f %f %f\n",
			curVert->pos.x(), curVert->pos.y(), curVert->pos.z());
	}
	for (auto face : curMesh->faceSet)
	{
		vector<int> vid;
		he_t* he = face->he;
		he_t* curHE = he;
		fprintf(OBJ_File, "f");
		do
		{
			fprintf(OBJ_File, " %d/0/0", curHE->v->index + 1);
			curHE = curHE->next;
		} while (curHE != he);

		fprintf(OBJ_File, "\n");
	}

	fclose(OBJ_File);
}

