#include "MeshLoader.h"

MeshLoader::MeshLoader(const string &filename)
{
	/************************************************************************/
	/* C Stype                                                              */
	/************************************************************************/
	errno_t err;
	FILE *file;
	err = fopen_s(&file, filename.c_str(), "r");
	if (err != 0)
	{
		cout << "Unable to open the file!" << endl;
		return;
	}
	while (true)
	{
		char lineHeader[128];
		int res = fscanf_s(file, "%s", &lineHeader, _countof(lineHeader));
		if (res == EOF)
		{
			break;
		}
		if (strcmp(lineHeader, "v") == 0)
		{
			double x, y, z;
			fscanf_s(file, " %lf %lf %lf\n", &x, &y, &z);
			vertices.push_back(QVector3D(x, y, z));
		}
		else if (strcmp(lineHeader, "vt") == 0)
		{
			double x, y;
			fscanf_s(file, " %lf %lf\n", &x, &y);
			uvs.push_back(QVector2D(x, y));
		}
		else if (strcmp(lineHeader, "vn") == 0)
		{
			double x, y, z;
			fscanf_s(file, " %lf %lf %lf\n", &x, &y, &z);
			normals.push_back(QVector3D(x, y, z));
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			QuadFaceIndex faceIndex;
			//int faceIndex[3][3];
			// Does not support empty uv or normal
			int iv, iuv, inorm;
			//fgets(lineHeader, _countof(lineHeader), file);

			fscanf_s(file, " %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n",
				&faceIndex.vtx[0], &faceIndex.uv[0], &faceIndex.n[0],
				&faceIndex.vtx[1], &faceIndex.uv[1], &faceIndex.n[1],
				&faceIndex.vtx[2], &faceIndex.uv[2], &faceIndex.n[2],
				&faceIndex.vtx[3], &faceIndex.uv[3], &faceIndex.n[3]);

			fids.push_back(faceIndex);
		}
		else
		{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}
	fclose(file);
}

MeshLoader::~MeshLoader()
{
}
