#ifndef __HDS_MESH__
#define __HDS_MESH__

#include <QVector3D>

#include "common.h"
#include "hds_halfedge.h"
#include "hds_vertex.h"
#include "hds_face.h"
#include "MeshLoader.h"

typedef float vbo_t;
class HDS_Mesh
{
public:
	typedef QVector3D point_t;
	typedef HDS_Face face_t;
	typedef HDS_Vertex vert_t;
	typedef HDS_HalfEdge he_t;
	typedef HDS_Mesh mesh_t;


	enum ElementType {
		Face = 0,
		Edge,
		Vertex
	};

	//HDS_Mesh();
	HDS_Mesh(const string &filename);
	HDS_Mesh(const HDS_Mesh &mesh);
	~HDS_Mesh();

	HDS_Mesh operator=(const HDS_Mesh& rhs);

	void reIndexing();
	void reIndexVert();
	void reIndexHE();
	void reIndexFace();

	void validate() const;
	//void printInfo(const string &msg = "");
	//void printMesh(const string &msg = "");
	void releaseMesh();

	void collapse(HDS_HalfEdge* he, const QVector3D &newPos,
		unordered_set<HDS_HalfEdge*> *touchingEdges = nullptr,
		unordered_set<HDS_HalfEdge*> *invalidEdges = nullptr,
		unordered_set<HDS_Face*> *invalidFaces = nullptr);
	void extrude(HDS_Face* face);

	void exportVBO(int &size,
		vbo_t** vtx_array = nullptr, vbo_t** uv_array = nullptr,
		vbo_t** norm_array = nullptr, int** idx_array = nullptr) const;
	void exportVBO(vector<float>* vtx_array = nullptr,
		vector<float>* uv_array = nullptr,
		vector<float>* norm_array = nullptr) const;
	void exportIndexedVBO(vector<float>* vtx_array = nullptr,
		vector<float>* uv_array = nullptr,
		vector<float>* norm_array = nullptr,
		vector<uint>* idx_array = nullptr,
		int order = 0) const;

private:
	bool validateVertex(vert_t *v) const;
	bool validateFace(face_t *f) const;
	bool validateEdge(he_t *e) const;
private:
	friend class Subdivision;
	friend class Simplification;
	friend class HairMesh;
	friend class HairMeshLayer;
	friend class OGLViewer;
private:
	unordered_set<he_t*> heSet;
	unordered_set<face_t*> faceSet;
	unordered_set<vert_t*> vertSet;

	unordered_map<int, vert_t*> vertMap;
	unordered_map<int, he_t*> heMap;
	unordered_map<int, face_t*> faceMap;

	//////////////////////////////////////////////////////////////////////////
	unordered_set<QVector3D*> snSet;
	unordered_set<QVector2D*> suvSet;
	unordered_map<int, QVector3D*> snMap;
	unordered_map<int, QVector2D*> suvMap;
};
/*
inline ostream& operator<<(ostream &os, const HDS_Vertex& v)
{
	os << v.index
		<< ": (" << v.pos.x() << ", " << v.pos.y() << ", " << v.pos.z() << ")"
		<< "\t"
		<< v.he;
	return os;
}

inline ostream& operator<<(ostream &os, const HDS_HalfEdge& e)
{
	os << e.index << "::"

		<< " prev: " << e.prev->index
		<< " next: " << e.next->index
		<< " flip: " << e.flip->index
		<< " v: " << e.v->index
		<< " f:" << e.f->index;
	return os;
}


inline ostream& operator<<(ostream &os, const HDS_Face& f)
{
	os << "face #" << f.index << " " << f.n << endl;
	HDS_HalfEdge *he = f.he;
	HDS_HalfEdge *curHE = he;
	do
	{
		os << "(" << curHE->index << ", " << curHE->v->index << ") ";
		curHE = curHE->next;
	} while( curHE != he );
	return os;
}*/

#endif // __HDS_MESH__