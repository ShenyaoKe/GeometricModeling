#include "hairMesh.h"


LayeredHairMesh::LayeredHairMesh(const HDS_Face* src, bool deepSet)
	: root(src)
	, vertice(src->corners())
	//, color(rand() % 256, rand() % 256, rand() % 256)
{
	seg = vertice.size();
	if (deepSet)
	{
		for (auto v : vertice)
		{
			points.push_back(v->pos);
		}
		color.setHsv(rand() % 360, 180, 255);
	}
	
}

LayeredHairMesh::~LayeredHairMesh()
{
	root = nullptr;
	vertice.clear();
	points.clear();
}

HairMesh::HairMesh(const HDS_Mesh* src, const char *filename)
	: ref_mesh(src)
{
	if (filename == nullptr)
	{
		return;
	}
	// Load from file
	FILE *HMS_File;
	errno_t err = fopen_s(&HMS_File, filename, "r");
	if (err)
	{
		printf("Can't read file %s!\n", filename);
		return;
	}
	float x, y, z;
	int fid;
	LayeredHairMesh* curLayer = nullptr;
	while (true)
	{
		char lineHeader[128];
		int res = fscanf_s(HMS_File, "%s", &lineHeader, _countof(lineHeader));
		if (res == EOF)
		{
			break;
		}
		if (strcmp(lineHeader, "L") == 0)
		{
			fscanf_s(HMS_File, " F%d\n", &fid);
			LayeredHairMesh* layer = new LayeredHairMesh(ref_mesh->faceMap.at(fid), false);
			curLayer = layer;
			layers.push_back(layer);
		}
		else if (strcmp(lineHeader, "c") == 0)
		{
			char color_str[8];
			fscanf_s(HMS_File, " %s\n", &color_str, _countof(color_str));
			curLayer->color = QColor(color_str);
		}
		else if (strcmp(lineHeader, "v") == 0)
		{

			fscanf_s(HMS_File, " %f %f %f\n", &x, &y, &z);
			curLayer->points.push_back(QVector3D(x, y, z));
		}
		else//if (strcmp(lineHeader, "#") == 0)
		{
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, HMS_File);
		}
	}
	fclose(HMS_File);
}

HairMesh::~HairMesh()
{
	for (auto layer : layers)
	{
		delete layer;
	}
	layers.clear();
}

void HairMesh::push_back(LayeredHairMesh* layer)
{
	layers.push_back(layer);
}

int HairMesh::sizeAtStroke(int i) const
{
	auto curLayer = layers[i];
	return curLayer->points.size() / curLayer->seg;
}

bool HairMesh::empty() const
{
	return layers.size() == 0;
}

void HairMesh::save(const char* filename) const
{
	FILE *HMS_File;
	errno_t err = fopen_s(&HMS_File, filename, "w");
	if (err)
	{
		printf("Can't write to file %s!\n", filename);
		return;
	}
	fprintf(HMS_File, "#Hair Mesh\n");
	for (int i = 0; i < layers.size(); i++)
	{
		auto curLayer = layers[i];
		// Write stroke ref face id
		fprintf(HMS_File, "L F%d\n", curLayer->root->index);

		//Write stroke colors
		fprintf(HMS_File, "c %s\n", curLayer->color.name().toStdString().c_str());

		for (int i = 0; i < curLayer->points.size(); i++)
		{
			auto& curPt = curLayer->points[i];
			fprintf(HMS_File, "v %f %f %f\n", curPt.x(), curPt.y(), curPt.z());
		}
	}

	fclose(HMS_File);
}

void HairMesh::initialSimulation()
{
	for (auto layer : layers)
	{
		auto& pts = layer->points;
		layer->simPts.clear();
		layer->simPts = pts;
		layer->simVel.clear();
		layer->simVel.insert(layer->simVel.end(), pts.size(), QVector3D());
		layer->restLen.clear();
		layer->restLen.insert(layer->restLen.end(), pts.size(), 0);
		//layer->restAng.insert(layer->restAng.end(), pts.size(), 0);
		for (int i = 4; i < pts.size(); i++)
		{
			layer->restLen[i] = (pts[i] - pts[i - 4]).length();
			//layer->restAng[i] = 
		}
	}
}

void HairMesh::simulate()
{
	for (auto layer : layers)
	{
		layer->simulate();
	}
}

void HairMesh::assignCollision(const KdTreeAccel* kdtree)
{
	acceltree = kdtree;
	for (auto layer : layers)
	{
		layer->acceltree = kdtree;
	}
}

void HairMesh::exportIndexedVBO(
	vector<float>* vtx_array, vector<uint>* idx_array,
	vector<uint>* vtx_offset_array, vector<uint>* idx_offset_array,
	vector<float>* layer_color, vector<float>* sim_vtx_array) const
{
	int layerSize = layers.size();
	int bufferSize = 0;
	if (vtx_array != nullptr)
	{
		vtx_array->clear();
		if (vtx_offset_array != nullptr)
		{
			vtx_offset_array->clear();
			vtx_offset_array->reserve(layerSize + 1);
		}
		if (idx_offset_array)
		{
			idx_offset_array->clear();
			idx_offset_array->reserve(layerSize + 1);
		}
		
		for (auto layer : layers)
		{
			vtx_offset_array->push_back(bufferSize);
			bufferSize += layer->points.size();
		}
		vtx_offset_array->push_back(bufferSize);
		vtx_array->reserve(bufferSize * 3);
	}
	if (sim_vtx_array != nullptr)
	{
		sim_vtx_array->clear();
		sim_vtx_array->reserve(bufferSize * 3);
	}
	if (idx_array != nullptr)
	{
		idx_array->clear();
	}
	
	idx_offset_array->push_back(0);
	for (int i = 0; i < layerSize; i++)
	{
		auto layer = layers[i];
		layer->exportIndexedVBO((*vtx_offset_array)[i], vtx_array, idx_array, sim_vtx_array);
		idx_offset_array->push_back(idx_array->size());
	}
	if (layer_color != nullptr)
	{
		layer_color->clear();
		layer_color->reserve(layerSize * 3);
		for (int i = 0; i < layerSize; i++)
		{
			auto& curColor = layers[i]->color;
			layer_color->push_back(static_cast<float>(curColor.redF()));
			layer_color->push_back(static_cast<float>(curColor.greenF()));
			layer_color->push_back(static_cast<float>(curColor.blueF()));
		}
	}
}

void LayeredHairMesh::extrude(double val)
{
	int curSize = points.size();
	int v0 = curSize - seg;
	int v1 = v0 + 1;
	int v2 = v1 + 1;
	int v3 = v2 + 1;
	QVector3D normal = val * QVector3D::crossProduct(
		points[v2] - points[v0], points[v3] - points[v1]).normalized();
	for (int i = curSize - seg; i < curSize; i++)
	{
		points.push_back(points[i] + normal);
	}
}

void LayeredHairMesh::translate(int lv, const QVector3D &dir, int mode)
{
	if (lv < 1)
	{
		return;
	}
	lv = min(lv, (int)points.size() / seg - 1);
	lv *= seg;

	for (int i = lv; i < lv + seg; i++)
	{
		points[i] += dir;
	}
}

void LayeredHairMesh::scale(int lv, double xval, double yval, double zval)
{
	if (lv < 1)
	{
		return;
	}
	lv = min(lv, (int)points.size() / seg - 1);
	lv *= seg;

	QVector3D center;
	for (int i = lv; i < lv + seg; i++)
	{
		center += points[i];
	}
	center /= seg;
	QVector3D normal = QVector3D::crossProduct(
		points[lv] - center, points[lv + 1] - center).normalized();
	QVector3D dx = (points[lv + 1] - points[lv]).normalized();
	QVector3D dy = QVector3D::crossProduct(normal, dx);

	for (int i = lv; i < lv + seg; i++)
	{
		points[i] = QVector3D::dotProduct(points[i] - center, dx) * yval * dx
			+ QVector3D::dotProduct(points[i] - center, dy) * xval * dy
			+ QVector3D::dotProduct(points[i] - center, normal) * xval * normal
			+ center;
	}
}

void LayeredHairMesh::rotate(int lv, double val, int mode)
{
	if (lv < 1)
	{
		return;
	}
	lv = min(lv, (int)points.size() / seg - 1);
	lv *= seg;

	QMatrix4x4 rot;
	QVector3D center;
	for (int i = lv; i < lv + seg; i++)
	{
		center += points[i];
	}
	center /= seg;
	QVector3D normal = QVector3D::crossProduct(
		points[lv] - center, points[lv + 1] - center).normalized();
	QVector3D dx = (points[lv + 1] - points[lv]).normalized();
	QVector3D dy = QVector3D::crossProduct(normal, dx);
	switch (mode)
	{
	case 0:// normal
	{

		rot.rotate(val, normal);
		break;
	}
	case 1:// Y dir
	{
		rot.rotate(val, dy);
		break;
	}
	case 2:// Y dir
	{
		rot.rotate(val, dx);
		break;
	}
	default:
		break;
	}
	
	for (int i = lv; i < lv + seg; i++)
	{
		points[i] = rot * (points[i] - center) + center;
	}
}

void LayeredHairMesh::remove(int lv)
{
	auto curPt = points.begin() + lv * seg;
	if (lv > 0)
	{

		points.erase(curPt, curPt + seg);
	}
}

void LayeredHairMesh::exportIndexedVBO(int offset,
	vector<float>* vtx_array, vector<uint>* idx_array,
	vector<float>* sim_vtx_array) const
{	
	if (vtx_array != nullptr)
	{
		for (auto pt : points)
		{
			vtx_array->push_back(pt.x());
			vtx_array->push_back(pt.y());
			vtx_array->push_back(pt.z());
		}
	}
	if (idx_array != nullptr)
	{
		// If no mesh extruded
		for (int i = seg + offset; i < points.size() + offset; i++)
		{
			if ((i - offset) % seg == 0)
			{
				idx_array->push_back(static_cast<uint>(i));
				idx_array->push_back(static_cast<uint>(i + seg - 1));
				idx_array->push_back(static_cast<uint>(i - 1));
				idx_array->push_back(static_cast<uint>(i - seg));
			}
			else
			{
				idx_array->push_back(static_cast<uint>(i));
				idx_array->push_back(static_cast<uint>(i - 1));
				idx_array->push_back(static_cast<uint>(i - 1 - seg));
				idx_array->push_back(static_cast<uint>(i - seg));
			}

		}

		for (int i = points.size() - seg + offset; i < points.size() + offset; i++)
		{
			idx_array->push_back(i);
		}
	}
	if (sim_vtx_array != nullptr)
	{
		for (auto pt : simPts)
		{
			sim_vtx_array->push_back(pt.x());
			sim_vtx_array->push_back(pt.y());
			sim_vtx_array->push_back(pt.z());
		}
	}

	
}

void LayeredHairMesh::externalUpdate(vector<QVector3D> &state)
{
	int compSize = state.size() / 2;
	for (int i = 4; i < compSize; i++)
	{
		state[i + compSize] += (gravity + windforce) * timestep;
		state[i] += 0.5 * state[i + compSize] * timestep;
	}
}

void LayeredHairMesh::internalUpdate(vector<QVector3D> &state)
{
	//K1
	auto K1 = internalForces(state) * timestep;
	auto K2 = internalForces(state + K1 * (timestep * 0.5)) * timestep;
	auto K3 = internalForces(state + K2 * (timestep * 0.5)) * timestep;
	auto K4 = internalForces(state + K3 * timestep) * timestep;
	double dem = 1.0 / 6.0;
	state = state + K1 * dem + K2 * (dem * 2) + K3 * (dem * 2) + K4 * dem;
	int compSize = state.size() / 2;
	for (int i = 4; i < compSize; i++)
	{
		simPts[i] = state[i];
		simVel[i] = state[i + compSize];
	}
}

vector<QVector3D> LayeredHairMesh::internalForces(const vector<QVector3D> &state) const
{
	vector<QVector3D> ifs(state.size(), QVector3D());
	float ks(5), kd(2), ko(1);
	int compSize = state.size() / 2;
	float lenSeg = compSize / seg;
	for (int i = 4; i < compSize; i ++)
	{
		// Spring restriction
		QVector3D dir = state[i - 4] - state[i];
		float deltaLen = dir.length() - restLen[i];
		dir.normalize();// *deltaLen;
		QVector3D fs = ks * dir * deltaLen;
		QVector3D fd = kd *
			QVector3D::dotProduct(
			state[i - 4 + compSize] - state[i + compSize], dir) * dir;

		// Constrant to original mesh
		float segU = (1 - (float)(i / seg) / lenSeg);
		QVector3D distDir = (points[i] - state[i]) * ko;

		ifs[i] = state[i + compSize];
		ifs[i + compSize] += fs + fd - state[i + compSize] * 0.1 + distDir;
		ifs[i - 4 + compSize] -= fs + fd;
	}
	for (int i = 0; i < 4; i++)
	{
		ifs[i] = QVector3D();
		ifs[i + compSize] = QVector3D();
	}
	return ifs;
}

void LayeredHairMesh::collisionDetect(vector<QVector3D> &state)
{
	int compSize = state.size() / 2;
	for (int i = 4; i < compSize; i++)
	{
		auto& curPos = simPts[i];
		auto& nextPos = state[i];
		auto dir = nextPos - curPos;
		double pathLen = dir.length();
		dir /= pathLen;
		
		if ((nextPos - sphere).lengthSquared() < radSq)
		{
			auto norm = (nextPos - sphere).normalized();
			state[i] = norm * radius * 1.01 + sphere;
			state[i + compSize] -= QVector3D::dotProduct(norm, state[i + compSize]) * 2 * norm;
		}
		/*Ray inray(Point3D(curPos.x(), curPos.y(), curPos.z()), Point3D(dir.x(), dir.y(), dir.z()));
		DifferentialGeometry queryPt;
		double thit = INFINITY, eps;
		if (acceltree->hit(inray, &queryPt, &thit, &eps))
		{
			if (thit < pathLen)
			{
				state[i] = curPos;
			}
		}*/
		
		
	}
}

void LayeredHairMesh::simulate()
{
	auto state = simPts;
	state.insert(state.end(), simVel.begin(), simVel.end());
	externalUpdate(state);
	internalUpdate(state);
	collisionDetect(state);

	int compSize = simPts.size();
	simPts.clear();
	simPts.insert(simPts.end(), state.begin(), state.begin() + compSize);
	simVel.clear();
	simVel.insert(simVel.end(), state.begin() + compSize, state.end());
}
