#include "Subdivision.h"



Subdivision::Subdivision(uint lv, const HDS_Mesh* origin)
	: level(lv)
	, origin_mesh(origin)
{
	const mesh_t* curMesh = origin_mesh;
	for (int i = 0; i < level; i++)
	{
		mesh_t* subd_ret = subdivide(curMesh);
		cout << "Subdivided at level " << i + 1 << "\n\t";
		subd_mesh.push_back(subd_ret);
		curMesh = subd_ret;
	}
}

Subdivision::~Subdivision()
{
}

uint Subdivision::getLevel() const
{
	return level;
}

void Subdivision::exportIndexedVBO(int lv,
	vector<float>* vtx_array, vector<float>* uv_array,
	vector<float>* norm_array, vector<uint>* idx_array)
{
	if (lv < 1)// 0 or negative value
	{
		origin_mesh->exportIndexedVBO(vtx_array, uv_array, norm_array, idx_array);
	}
	else // 1 or bigger
	{
		lv = min(lv - 1, (int)subd_mesh.size() - 1);
		subd_mesh[lv]->exportIndexedVBO(vtx_array, uv_array, norm_array, idx_array);
	}
}

mesh_t* Subdivision::subdivide(const mesh_t* origin)
{
	mesh_t* ret = new mesh_t(*origin);
	
#ifdef _DEBUG
	clock_t subdiv_start, start_time, end_time;//Timer
	subdiv_start = start_time = clock();
#endif
	unordered_set<vert_t*> fcSet; 
	unordered_set<vert_t*> ecSet;
	unordered_map<face_t*, vert_t*> fcMap;
	unordered_map<he_t*, vert_t*> ecMap;
	fcSet.reserve(ret->faceSet.size());
	ecSet.reserve(ret->heSet.size() / 2);
	fcMap.reserve(ret->faceSet.size());
	ecMap.reserve(ret->heSet.size());
	int index = ret->vertSet.size();

	// Insert center face
	for (auto face : ret->faceSet)
	{
		vert_t* fc = new vert_t(face->center());
		fc->index = index++;// HDS_Vertex::assignIndex();
		fcSet.insert(fc);
		fcMap.insert(make_pair(face, fc));
		ret->vertMap.insert(make_pair(fc->index, fc));
	}
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tGenerate Face Center:\t" << end_time - start_time << "ms\n";
	start_time = clock();
#endif
	// Insert vertex on edge
	auto unvisitedEdge = ret->heMap;
	vector<bool> visited(ret->heSet.size(), false);
	for (auto edge : ret->heSet)
	{
		/*if (unvisitedEdge.find(edge->index) == unvisitedEdge.end())
		{
			// If visited, conitue loop
			continue;
		}
		unvisitedEdge.erase(edge->index);
		unvisitedEdge.erase(edge->flip->index);*/
		if (visited[edge->index])
		{
			continue;
		}
		visited[edge->index] = visited[edge->flip->index] = true;

		int neighbourCount = 2;
		QVector3D pos = edge->v->pos + edge->flip->v->pos;
		if (edge->f != nullptr)
		{
			pos += fcMap[edge->f]->pos;
			neighbourCount++;
		}
		if (edge->flip->f != nullptr)
		{
			pos += fcMap[edge->flip->f]->pos;
			neighbourCount++;
		}
		pos /= neighbourCount;

		vert_t* ec = new vert_t(pos);
		ec->index = index++;// HDS_Vertex::assignIndex();
		ec->he = edge;
		ecSet.insert(ec);
		ecMap.insert(make_pair(edge, ec));
		ecMap.insert(make_pair(edge->flip, ec));
		ret->vertMap.insert(make_pair(ec->index, ec));
	}
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tGenerate Edge Center:\t" << end_time - start_time << "ms\n";
	start_time = clock();
#endif
	// Catmull coefficiencies
	float catmull[2] = { -1, 4 };//{1, 2}
	// Modify original vertex
	for (auto vert : ret->vertSet)
	{
		vector<HDS_Vertex*> neighborFC, neighborEC;

		HDS_HalfEdge *curHE = vert->he;
		do
		{
			//neighbours.push_back(curHE->flip->v);
			neighborEC.push_back(ecMap[curHE]);
			neighborFC.push_back(fcMap[curHE->f]);
			curHE = curHE->flip->next;
		} while (curHE != vert->he);

		// Update position
		float size = neighborFC.size();
		vert->pos *= (size - 3.0) / size;

		QVector3D avgFC;
		for (auto v : neighborFC)
		{
			avgFC += v->pos;
		}
		vert->pos += catmull[0] * avgFC / static_cast<float>(neighborFC.size() * neighborFC.size());

		QVector3D avgEC;
		for (auto v : neighborEC)
		{
			avgEC += v->pos;
		}
		vert->pos += catmull[1] * avgEC / static_cast<float>(neighborEC.size() * neighborEC.size());
	}
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tMoving Original Position:\t" << end_time - start_time << "ms\n";
	start_time = clock();
#endif
	// insert edge center and connect edges
	unordered_set<he_t*> nheSet;
	nheSet.reserve(ecSet.size() * 4);
	for (auto v : ecSet)
	{
		he_t* he = v->he;
		he_t* hef = v->he->flip;
		vert_t* hev = he->v;
		vert_t* hefv = hef->v;

		he_t* nhe = new he_t;
		he_t* nhef = new he_t;
		nhe->index = HDS_HalfEdge::assignIndex();
		nhef->index = HDS_HalfEdge::assignIndex();
		nhe->v = nhef->v = v;
		/*auto heprev = he->prev;
		auto hefprev = hef->prev;
		nhe->next = he; nhe->prev = heprev;
		nhe->next->prev = nhe; nhe->prev->next = nhe;
		nhef->next = hef; nhe->prev = hefprev;
		nhef->next->prev = nhef; nhef->prev->next = nhef;
		/ *he->prev = heprev->next = nhe;
		hef->prev = hefprev->next = nhef;*/

		nhe->next = he; nhe->prev = he->prev; he->prev->next = nhe; he->prev = nhe;
		nhef->next = hef; nhef->prev = hef->prev; hef->prev->next = nhef; hef->prev = nhef;
		hev->he = nhef; hefv->he = nhe;//*/

		nhe->flip = hef;
		hef->flip = nhe;
		nhef->flip = he;
		he->flip = nhef;
		nhe->f = he->f;
		nhef->f = hef->f;
		nheSet.insert(nhe);
		nheSet.insert(nhef);
		ret->heMap.insert(make_pair(nhe->index, nhe));
		ret->heMap.insert(make_pair(nhef->index, nhef));
	}
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tInsert Edge Center:\t" << end_time - start_time << "ms\n";
	start_time = clock();
#endif
	// Insert face centers
	for (auto face_query : fcMap)
	{
		face_t* face = face_query.first;// face pointer
		vert_t* fc = face_query.second;// face center
		he_t* he = face->he;
		he_t* curHE = he;
		vector<he_t*> newHE;
		do 
		{
			/*face_t* newface = new face_t;
			newface->he = curHE;
			newface->index = HDS_Face::assignIndex();
			ret->faceSet.insert(newface);
			ret->faceMap.insert(make_pair(newface->index, newface));*/

			curHE = curHE->next;
			//vert_t* newv = curHE->v;

			he_t* nhe = new he_t;
			he_t* nhef = new he_t;
			nhe->index = HDS_HalfEdge::assignIndex();
			nhef->index = HDS_HalfEdge::assignIndex();
			nhe->flip = nhef; nhef->flip = nhe;

			nhe->v = fc;
			nhef->v = curHE->v;
			nhe->prev = curHE;
			nhef->next = curHE->next;

			//curHE->prev->prev->f = curHE->prev->f = curHE->f = nhe->f = newface;

			curHE = curHE->next;
			curHE->prev->next = nhe;
			curHE->prev = nhef;

			newHE.push_back(nhe);
			newHE.push_back(nhef);
			nheSet.insert(nhe);
			nheSet.insert(nhef);
			ret->heMap.insert(make_pair(nhe->index, nhe));
			ret->heMap.insert(make_pair(nhef->index, nhef));

		} while (curHE != he);
		fc->he = newHE.back();
		for (int i = 0; i < newHE.size(); i+=2)
		{
			int j = (i + newHE.size() - 1) % newHE.size();
			newHE[i]->next = newHE[j];
			newHE[j]->prev = newHE[i];
			face_t* newface = new face_t;
			newface->he = newHE[j];
			newface->index = HDS_Face::assignIndex();
			ret->faceSet.insert(newface);
			ret->faceMap.insert(make_pair(newface->index, newface));

			he = newHE[i];
			curHE = he;
			do 
			{
				curHE->f = newface;
				curHE = curHE->next;
			} while (curHE != he);

		}
		newHE.back()->f = newHE.back()->next->f;
		ret->faceSet.erase(face);
		ret->faceMap.erase(face->index);
		delete face;
	}
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tInsert Faces:\t" << end_time - start_time << "ms\n";
	start_time = clock();
#endif
	//ret->validate();
	// Finalize return mesh
	ret->heSet.insert(nheSet.begin(), nheSet.end());
	ret->vertSet.insert(ecSet.begin(), ecSet.end());
	ret->vertSet.insert(fcSet.begin(), fcSet.end());
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tMerging:\t" << end_time - start_time << "ms\n";
	start_time = clock();
#endif
	//ret->reIndexing();
#ifdef _DEBUG
	end_time = clock();
	cout << "\t\tIndexing:\t" << end_time - start_time << "ms\n";
	cout << "\t\tSubdivision Time:\t" << (float)(end_time - subdiv_start) / CLOCKS_PER_SEC << "s" << endl;//Timer
	start_time = clock();
#endif

	return ret;
}
