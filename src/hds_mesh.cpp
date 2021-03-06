#include "hds_mesh.h"

using namespace std;
/*
#ifndef _ENABLE_CLOCK
#define  _ENABLE_CLOCK 0
#endif*/
HDS_Mesh::HDS_Mesh(const char* filename)
{
#ifdef _DEBUG
	clock_t start_time, end_time;//Timer
	start_time = clock();
#endif

	MeshLoader meshIndex(filename);
#ifdef _DEBUG
	end_time = clock();
	cout << "Loading File " << filename << " as index Time:" << (float)(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;//Timer
	start_time = clock();
#endif
	// Alias for clean code
	auto& vertices = meshIndex.vertices;
	auto& uvs = meshIndex.uvs;
	auto& normals = meshIndex.normals;

	// Generate HDS_Vetex
	for (int i = 0; i < vertices.size(); i++)
	{
		int pPtr = i * 3;
		vert_t* v = new vert_t(QVector3D(vertices[pPtr], vertices[pPtr + 1], vertices[pPtr + 2]));
		v->index = vert_t::assignIndex();
		vertSet.insert(v);
		vertMap.insert(make_pair(i, v));
	}
	/*for (int i = 0; i < normals.size(); i++)
	{
		QVector3D* sn = new QVector3D(normals[i]);
		snSet.insert(sn);
		snMap.insert(make_pair(i, sn));
	}
	for (int i = 0; i < uvs.size(); i++)
	{
		QVector2D* suv = new QVector2D(uvs[i]);
		suvSet.insert(suv);
		suvMap.insert(make_pair(i, suv));
	}*/
	
	// Generate HDS_Face
	map<pair<int, int>, he_t*> heUnassigned;
	auto unassignedVerts = vertMap;
	for (auto fid : meshIndex.polys)
	{
		face_t* face = new face_t;
		face->index = HDS_Face::assignIndex();
		faceSet.insert(face);
		faceMap.insert(make_pair(face->index, face));
		int vnum = face->vnum = fid->v.size();
		
		he_t* prevHE = nullptr;
		he_t* beginHE = nullptr;
		for (int i = 0; i < vnum; i++)
		{
			// Vertex attributes
			int curVID = fid->v[i] - 1;
			if (curVID < 0)
			{
				continue;
			}
			int nextVID = fid->v[(i + 1) % vnum] - 1;
			/*int curSNID = fid.n[i] - 1;
			int curSUVID = fid.uv[i] - 1;*/
			vert_t* curVertex = vertMap[curVID];
			/*QVector3D* curSN = snMap[curSNID];
			QVector2D* curSUV = suvMap[curSUVID];
			face->snMap.insert(make_pair(curVertex, curSN));
			face->suvMap.insert(make_pair(curVertex, curSUV));*/

			// Half-edge
			he_t* he = new he_t(vertMap[curVID], face);
			he->index = HDS_HalfEdge::assignIndex();	// Assign index
			he->v = vertMap[nextVID];
			if (unassignedVerts.find(curVID) != unassignedVerts.end())
			{
				unassignedVerts[curVID]->he = he;
			}
			vertMap[curVID]->he = he;
			heSet.insert(he);						// Add to set
			heMap.insert(make_pair(he->index, he));	// Add to map
			
			he->prev = prevHE;
			// For the first edge
			if (i == 0)
			{
				beginHE = he;
			}

			auto oppEdgePair = make_pair(nextVID, curVID);	// pair for flip half-edge
			auto heQueryRes = heUnassigned.find(oppEdgePair);
			if (heQueryRes != heUnassigned.end())
			{
				he->flip = heQueryRes->second;
				heQueryRes->second->flip = he;
				heUnassigned.erase(heQueryRes);
			}
			else
			{
				heUnassigned.insert(make_pair(make_pair(curVID, nextVID), he));
			}

			prevHE = he;
		}
		beginHE->prev = prevHE;
		face->he = prevHE;// The first edge point to first vertex
		// Assign next point to next half-edge
		prevHE = beginHE;
		do 
		{
			prevHE->prev->next = prevHE;
			prevHE = prevHE->prev;
		} while (prevHE != beginHE);
		//face->updateVerts();
		//face->pre_n = face->computeNormal();
	}
	for (auto he : heUnassigned)
	{
		HDS_HalfEdge* hef = new HDS_HalfEdge(he.second->prev->v,nullptr);
		he.second->flip = hef;
		hef->flip = he.second;
		hef->index = HDS_HalfEdge::assignIndex();
		heSet.insert(hef);
		heMap.insert(make_pair(hef->index, hef));
	}
	/*for (auto face : faceSet)
	{
		face->index = HDS_Face::assignIndex();
		faceMap.insert(make_pair(face->index, face));
	}*/
	//reIndexing();
#ifdef _DEBUG
	end_time = clock();
	cout << "Building HDS_Mesh " << filename << " Time:" << (float)(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;//Timer
	//start_time = clock();
#endif
}

HDS_Mesh::HDS_Mesh(const HDS_Mesh &other)
{
#ifdef _DEBUG
	clock_t copy_start, start_time, end_time;//Timer
	copy_start = start_time = clock();
#endif
	// copy the vertices set
	/*vertSet.clear();
	vertMap.clear();*/
	vertSet.reserve(other.vertSet.size());
	vertMap.reserve(other.vertMap.size());
	//vertMap = other.vertMap;
	for (auto v : other.vertSet)
	{
		/// he is not set for this vertex
		vert_t *nv = new vert_t(*v);
		vertSet.insert(nv);
		vertMap.insert(make_pair(v->index, nv));
		//vertMap[v->index] = nv;
	}
#ifdef _DEBUG
	end_time = clock();
	cout << "Copy Vertex Data from " << &other << " Time:" << (float)(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;//Timer
	start_time = clock();
#endif
	/*faceSet.clear();
	faceMap.clear();*/
	faceSet.reserve(other.faceSet.size());
	faceMap.reserve(other.faceMap.size());
	//faceMap = other.faceMap;
	for (auto f : other.faceSet)
	{
		// he is not set for this vertex
		face_t *nf = new face_t(*f);
		faceSet.insert(nf);
		faceMap.insert(make_pair(f->index, nf));
		//faceMap[f->index] = nf;
	}
#ifdef _DEBUG
	end_time = clock();
	cout << "Copy Face Data from " << &other << " Time:" << (float)(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;//Timer
	start_time = clock();
#endif
	/*heSet.clear();
	heMap.clear();*/
	heSet.reserve(other.heSet.size());
	heMap.reserve(other.heMap.size());
	//heSet = other.heSet;
	//heMap = other.heMap;
	for (auto he : other.heSet)
	{
		// face, vertex, prev, next, and flip are not set yet
		he_t *nhe = new he_t(*he);
		heSet.insert(nhe);
		heMap.insert(make_pair(he->index, nhe));
		//heMap[he->index] = nhe;
	}
#ifdef _DEBUG
	end_time = clock();
	cout << "Copy Edge Data from " << &other << " Time:" << (float)(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;//Timer
	start_time = clock();
#endif
	// fill in the pointers
	for (auto &he : heSet)
	{
		auto he_ref = other.heMap.at(he->index);
		//cout << he_ref->index << endl;
		he->flip = heMap.at(he_ref->flip->index);
		he->prev = heMap.at(he_ref->prev->index);
		he->next = heMap.at(he_ref->next->index);

		he->f = faceMap.at(he_ref->f->index);
		he->v = vertMap.at(he_ref->v->index);
	}

	// set the half edges for faces
	for (auto &f : faceSet)
	{
		auto f_ref = other.faceMap.at(f->index);
		f->he = heMap.at(f_ref->he->index);
	}

	// set the half edges for vertices
	for (auto &v : vertSet)
	{
		auto v_ref = other.vertMap.at(v->index);
		v->he = heMap.at(v_ref->he->index);
	}


#ifdef _DEBUG
	end_time = clock();
	cout << "Build connection from " << &other << " Time:" << (float)(end_time - start_time) / CLOCKS_PER_SEC << "s" << endl;//Timer
	cout << "Copy HDS_Mesh from " << &other << " Time:" << (float)(end_time - copy_start) / CLOCKS_PER_SEC << "s" << endl;//Timer
	//start_time = clock();
#endif
}

HDS_Mesh::~HDS_Mesh()
{
	releaseMesh();
}

bool HDS_Mesh::validateEdge(he_t *e) const
{
	if( heMap.find(e->index) == heMap.end() ) return false;
	if( e->flip->flip != e ) return false;
	if( e->next->prev != e ) return false;
	if( e->prev->next != e ) return false;
	if( e->f == nullptr ) return false;
	if( e->v == nullptr ) return false;
	if( faceSet.find(e->f) == faceSet.end() ) return false;
	if( vertSet.find(e->v) == vertSet.end() ) return false;
	return true;
}

bool HDS_Mesh::validateFace(face_t *f) const
{
	if( faceMap.find(f->index) == faceMap.end() ) return false;

	int maxEdges = 100;
	he_t *he = f->he;
	he_t *curHe = he;
	int edgeCount = 0;
	do {
		curHe = curHe->next;
		++edgeCount;
		if( edgeCount > maxEdges ) return false;
	} while( curHe != he );
	return true;
}

bool HDS_Mesh::validateVertex(vert_t *v) const
{
	if( vertMap.find(v->index) == vertMap.end() ) return false;

	int maxEdges =100;
	he_t *he = v->he;
	he_t *curHe = he;
	int edgeCount = 0;
	do {
		curHe = curHe->flip->next;
		++edgeCount;
		if( edgeCount > maxEdges ) return false;
	} while( curHe != he );
	return true;
}

void HDS_Mesh::reIndexing()
{
	reIndexVert();
	reIndexHE();
	reIndexFace();
}

void HDS_Mesh::reIndexVert()
{
	vertMap.clear();
	HDS_Vertex::resetIndex();
	for (auto vert : vertSet)
	{
		vert->index = HDS_Vertex::assignIndex();
		vertMap.insert(make_pair(vert->index, vert));
	}
}

void HDS_Mesh::reIndexHE()
{
	heMap.clear();
	HDS_HalfEdge::resetIndex();
	for (auto he : heSet)
	{
		he->index = HDS_HalfEdge::assignIndex();
		heMap.insert(make_pair(he->index, he));
	}
}

void HDS_Mesh::reIndexFace()
{
	faceMap.clear();
	HDS_Vertex::resetIndex();
	for (auto face : faceSet)
	{
		face->index = HDS_Face::assignIndex();
		faceMap.insert(make_pair(face->index, face));
	}
}

void HDS_Mesh::validate() const
{
	/// verify that the mesh has good topology, ie has loop
	for (auto v : vertSet) {
		if (!validateVertex(v)) {
			cout << "vertex #" << v->index << " is invalid." << endl;
		}
	}

	for (auto f : faceSet) {
		if (!validateFace(f)) {
			cout << "face #" << f->index << " is invalid." << endl;
		}
	}

	for (auto e : heSet) {
		if (!validateEdge(e)) {
			//validateEdge(e);
			cout << "half edge #" << e->index << " is invalid." << endl;
		}
	}
}

void HDS_Mesh::releaseMesh()
{
	/*for (auto vit = vertSet.begin(); vit != vertSet.end(); vit++)
	{
		if ((*vit) != nullptr)
			delete (*vit);
	}*/
	for (auto vert : vertSet)
	{
		delete vert;
	}
	vertSet.clear();

	/*for (auto fit = faceSet.begin(); fit != faceSet.end(); fit++)
	{
		if ((*fit) != nullptr)
			delete (*fit);
	}*/
	for (auto face : faceSet)
	{
		delete face;
	}
	faceSet.clear();

	/*for (auto heit = heSet.begin(); heit != heSet.end(); heit++)
	{
		if ((*heit) != nullptr)
			delete (*heit);
	}*/
	for (auto he : heSet)
	{
		delete he;
	}
	heSet.clear();

	HDS_HalfEdge::resetIndex();
	
	vertMap.clear();
	heMap.clear();
	faceMap.clear();

	for (auto sn : snSet)
	{
		delete sn;
	}
	snSet.clear(); snMap.clear();
	for (auto suv : suvSet)
	{
		delete suv;
	}
	suvSet.clear(); suvMap.clear();
}

void HDS_Mesh::collapse(HDS_HalfEdge* he, const QVector3D &newPos,
	unordered_set<HDS_HalfEdge*> *touchingEdges,
	unordered_set<HDS_HalfEdge*> *invalidEdges,
	unordered_set<HDS_Face*> *invalidFaces)
{
	unordered_set<HDS_HalfEdge*> touchings, invalids;

	// Triangular mesh only
	auto hef = he->flip;

	// Record invalid edges, which will be removed
	if (invalidEdges != nullptr)
	{
		invalidEdges->insert(he);
		invalidEdges->insert(he->next);
		invalidEdges->insert(he->prev);
		invalidEdges->insert(hef);
		invalidEdges->insert(hef->next);
		invalidEdges->insert(hef->prev);
	}
	if (invalidFaces != nullptr)
	{
		invalidFaces->insert(he->f);
		invalidFaces->insert(hef->f);
	}

	// Move vertex to new position
	he->v->pos = newPos;

	// Remove opposite vertex and re-assign it to he->v
	auto curHE = hef->next->flip;
	auto curV = he->v;
	curV->he = curHE->next;
	he->next->v->he = he->next->flip;
	hef->next->v->he = curHE;
	do 
	{
		curHE->v = curV;
		curHE = curHE->next->flip;
		touchings.insert(curHE);
	} while (curHE != he->prev);

	// Find touching edges on the other side
	if (touchingEdges != nullptr)
	{
		touchingEdges->insert(touchings.begin(), touchings.end());
		curHE = he->next->flip;
		do 
		{
			touchingEdges->insert(curHE);
			curHE = curHE->next->flip;
		} while (curHE != hef->prev);
	}

	// Assign flip edge
	HDS_HalfEdge::connectEdges(he->next->flip, he->prev->flip);
	HDS_HalfEdge::connectEdges(hef->next->flip, hef->prev->flip);
}

void HDS_Mesh::extrude(HDS_Face* face)
{
	if (face == nullptr)
	{
		cout << "Face doesn't exist!" << endl;
		return;
	}

	he_t* he = face->he;
	he_t* curHE = he;
	vector<HDS_HalfEdge*> oldEdges;
	vector<HDS_Vertex*> newVerts;
	//vector<HDS_Face*> newFaces;
	//vector<HDS_HalfEdge*> newEdges;
	do 
	{
		oldEdges.push_back(curHE);
		vert_t* v = new vert_t(*curHE->v);
		newVerts.push_back(v);
		curHE = curHE->next;
		v->he = curHE;
	} while (curHE != he);
	int num = oldEdges.size();
	int edgenum = num * 4;

	he_t* newEdges = new he_t[edgenum];
	face_t* newFaces = new face_t[num];

	for (int i = 0; i < num; i++)
	{
		auto& curHE = oldEdges[i];
		int init = i * 4;
		int curr = init;
		int prev = i * 4 + 3;
		int next = i * 4 + 1;
		newEdges[curr].v = newVerts[num - i];
		newEdges[next].v = curHE->flip->v;
		newEdges[next + 1].v = curHE->v;
		newEdges[prev].v = newVerts[i];

		HDS_HalfEdge::connectEdges(&newEdges[prev], &newEdges[(prev + 2) % edgenum]);

		// Connect original edges with new edges
		HDS_HalfEdge::connectEdges(&newEdges[next + 1], oldEdges[i]->flip);
		HDS_HalfEdge::connectEdges(&newEdges[curr], oldEdges[i]);

		// Link face to first he
		newFaces[i].he = &newEdges[curr];

		// Connect edge from begin to the end;
		for (int j = 0; j < 4; j++)
		{
			curr = init + j;
			next = init + (j + 1) % 4;
			newEdges[curr].next = &newEdges[next];
			newEdges[next].prev = &newEdges[curr];
			newEdges[curr].f = &newFaces[i];
		}
		oldEdges[i]->v = newVerts[i];
	}

	// Insert vert, edge, face to mesh
	for (int i = 0; i < num; i++)
	{
		
	}
}

void HDS_Mesh::exportVBO(int &size,
	vbo_t** vtx_array, vbo_t** uv_array,
	vbo_t** norm_array, int** idx_array) const
{
	vbo_t *verts(nullptr), *texcoord(nullptr), *nms(nullptr);
	int *uids(nullptr);

	bool has_vert(false), has_texcoord(false), has_normal(false), has_uid(false);
	size = this->faceSet.size() * 2;
	if (vtx_array != nullptr)
	{
		*vtx_array = new vbo_t[size * 9];
		verts = *vtx_array;
		has_vert = true;
	}
	if (uv_array != nullptr)
	{
		*uv_array = new vbo_t[size * 6];
		texcoord = *uv_array;
		has_texcoord = true;
	}
	if (norm_array != nullptr)
	{
		*norm_array = new vbo_t[size * 9];
		nms = *norm_array;
		has_normal = true;
	}
	if (idx_array != nullptr)
	{
		*idx_array = new int[size * 3];
		uids = *idx_array;
		has_uid = true;
	}

	for (auto f : faceSet)
	{
		int idx[6] = { 0, 1, 2, 2, 3, 0 };
		
		vector<HDS_Vertex*> triangulatedVerts;
		//auto cur_fid = this->fids[i];
		if (true)//(has_vert)
		{
			for (int i = 0; i < 6; i++)
			{
				auto cur_vtx = f->v[idx[i]];
				triangulatedVerts.push_back(cur_vtx);
				*verts++ = cur_vtx->x();
				*verts++ = cur_vtx->y();
				*verts++ = cur_vtx->z();
			}
		}
		if (has_texcoord)
		{
			for (int i = 0; i < 6; i++)
			{
				auto cur_texcoord = f->suvMap[triangulatedVerts[i]];
				*texcoord++ = cur_texcoord->x();
				*texcoord++ = cur_texcoord->y();
			}
		}
		if (has_normal)
		{

			for (int i = 0; i < 6; i++)
			{
				auto cur_normal = f->snMap[triangulatedVerts[i]];
				*nms++ = cur_normal->x();
				*nms++ = cur_normal->y();
				*nms++ = cur_normal->z();
			}
		}
		/*if (has_uid)
		{
			*uids++ = this->index + i;
			*uids++ = this->index + i;
			*uids++ = this->index + i;
		}*/
	}
}

void HDS_Mesh::exportVBO(
	vector<float>* vtx_array,
	vector<float>* uv_array,
	vector<float>* norm_array) const
{
	bool has_vert(false), has_texcoord(false), has_normal(false);

	if (vtx_array != nullptr)
	{
		vtx_array->clear();
		vtx_array->reserve(vertSet.size() * 3);
		has_vert = true;
	}

	for (auto face : faceSet)
	{
		he_t* he = face->he;
		he_t* curHE = he;
		do
		{
			auto& point = curHE->v->pos;
			vtx_array->push_back(point.x());
			vtx_array->push_back(point.y());
			vtx_array->push_back(point.z());
			curHE = curHE->next;
		} while (curHE != he);
	}
}

void HDS_Mesh::exportIndexedVBO(
	vector<float>* vtx_array, vector<float>* uv_array,
	vector<float>* norm_array, vector<uint>* idx_array, int order) const
{
	bool has_vert(false), has_texcoord(false), has_normal(false), has_uid(false);

	if (vtx_array != nullptr)
	{
		vtx_array->clear();
		vtx_array->reserve(vertSet.size() * 3);
		has_vert = true;
	}
	/*if (this->fids[0]->uv >= 0 && uv_array != nullptr)
	{
	*uv_array = new vbo_t[size * 6];
	texcoord = *uv_array;
	has_texcoord = true;
	}
	if (this->fids[0]->n >= 0 && norm_array != nullptr)
	{
	*norm_array = new vbo_t[size * 9];
	nms = *norm_array;
	has_normal = true;
	}*/
	if (idx_array != nullptr)
	{
		idx_array->clear();
		idx_array->reserve(faceSet.size() * 3);
		has_uid = true;
	}

	for (int i = 0; i < vertSet.size(); i++)
	{
		auto point = vertMap.at(i);

		vtx_array->push_back(point->x());
		vtx_array->push_back(point->y());
		vtx_array->push_back(point->z());
	}
	// Triangle Fans
	/*for (auto face : faceSet)
	{
		he_t* he = face->he;
		he_t* curHE = he->next;
		do 
		{
			idx_array->push_back(static_cast<uint>(curHE->v->index));
			idx_array->push_back(static_cast<uint>(curHE->next->v->index));
			idx_array->push_back(static_cast<uint>(he->v->index));
			curHE = curHE->next;
		} while (curHE->next != he);
	}*/
	if (order == 0)// index independent
	{
		for (auto face : faceSet)
		{
			he_t* he = face->he;
			he_t* curHE = he;
			do
			{
				idx_array->push_back(static_cast<uint>(curHE->v->index));
				curHE = curHE->next;
			} while (curHE != he);
		}
	}
	else// Index dependent
	{
		for (int i = 0; i < faceMap.size() ; i++)
		{
			face_t* face = faceMap.at(i);
			he_t* he = face->he;
			he_t* curHE = he;
			do
			{
				idx_array->push_back(static_cast<uint>(curHE->v->index));
				curHE = curHE->next;
			} while (curHE != he);
		}
	}
	
}