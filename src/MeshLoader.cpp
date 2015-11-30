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
			//QuadFaceIndex faceIndex;
			int idxBuf[12] = {};
			// Does not support empty uv or normal

			fscanf_s(file, " %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n",
				&idxBuf[0], &idxBuf[1], &idxBuf[2],
				&idxBuf[3], &idxBuf[4], &idxBuf[5],
				&idxBuf[6], &idxBuf[7], &idxBuf[8],
				&idxBuf[9], &idxBuf[10], &idxBuf[11]);
			if (idxBuf[9] == 0)
			{
				fids.push_back(FaceIndex(idxBuf, 3));
			}
			else
			{
				fids.push_back(FaceIndex(idxBuf, 4));
			}
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
