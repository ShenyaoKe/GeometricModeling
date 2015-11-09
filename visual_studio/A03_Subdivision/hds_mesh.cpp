#include "hds_mesh.h"

using namespace std;

HDS_Mesh::HDS_Mesh(const string &filename)
{
	MeshLoader meshIndex(filename);
	// Alias for clean code
	auto& vertices = meshIndex.vertices;
	auto& uvs = meshIndex.uvs;
	auto& normals = meshIndex.normals;

	// Generate HDS_Vetex
	for (int i = 0; i < vertices.size(); i++)
	{
		vert_t* v = new vert_t(vertices[i]);
		v->index = i;
		vertSet.insert(v);
		vertMap.insert(make_pair(i, v));
	}
	for (int i = 0; i < normals.size(); i++)
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
	}
	
	// Generate HDS_Face
	map<pair<int, int>, he_t*> heUnassigned;
	auto unassignedVerts = vertMap;
	for (auto f : meshIndex.fids)
	{
		face_t* face = new face_t;
		face->index = HDS_Face::assignIndex();
		faceSet.insert(face);
		faceMap.insert(make_pair(face->index, face));
		face->vnum = 4;
		
		he_t* prevHE = nullptr;
		he_t* beginHE = nullptr;
		for (int i = 0; i < 4; i++)
		{
			// Vertex attributes
			int curVID = f.vtx[i] - 1;
			int nextVID = f.vtx[(i + 1) % 4] - 1;
			int curSNID = f.n[i] - 1;
			int curSUVID = f.uv[i] - 1;
			vert_t* curVertex = vertMap[curVID];
			QVector3D* curSN = snMap[curSNID];
			QVector2D* curSUV = suvMap[curSUVID];
			face->snMap.insert(make_pair(curVertex, curSN));
			face->suvMap.insert(make_pair(curVertex, curSUV));

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
		face->updateVerts();
		face->pre_n = face->computeNormal();
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
}

HDS_Mesh::~HDS_Mesh()
{
	releaseMesh();
}

bool HDS_Mesh::validateEdge(he_t *e)
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

bool HDS_Mesh::validateFace(face_t *f)
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

bool HDS_Mesh::validateVertex(vert_t *v)
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

void HDS_Mesh::validate()
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
			cout << "half edge #" << e->index << " is invalid." << endl;
		}
	}
}

void HDS_Mesh::releaseMesh()
{
	for (auto vit = vertSet.begin(); vit != vertSet.end(); vit++)
	{
		if ((*vit) != nullptr)
			delete (*vit);
	}
	vertSet.clear();

	for (auto fit = faceSet.begin(); fit != faceSet.end(); fit++)
	{
		if ((*fit) != nullptr)
			delete (*fit);
	}
	faceSet.clear();

	for (auto heit = heSet.begin(); heit != heSet.end(); heit++)
	{
		if ((*heit) != nullptr)
			delete (*heit);
	}
	heSet.clear();
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