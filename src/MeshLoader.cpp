#include "MeshLoader.h"

MeshLoader::MeshLoader(const char* filename)
{
//#ifdef _DEBUG
	clock_t startTime = clock();
//#endif // _DEBUG
	//load(filename);
	load_from_file(filename);
//#ifdef _DEBUG
	cout << "Loding Time of OBJ File " << filename << ":\t" << (float)(clock() - startTime) / CLOCKS_PER_SEC << "s" << endl;//Timer
//#endif // _DEBUG
}

/*
MeshLoader::MeshLoader(const string &filename)
{
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
		else if (strcmp(lineHeader, "f") == 0)
		{
			//QuadFaceIndex faceIndex;
			int idxBuf[12] = {};
			// Does not support empty uv or normal

			fscanf_s(file, " %d %d %d\n",
				&idxBuf[0], &idxBuf[3], &idxBuf[6]);
			fids.push_back(FaceIndex(idxBuf, 3));
			
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
*/


MeshLoader::~MeshLoader()
{
}

void MeshLoader::release()
{
	vertices.clear();
	uvs.clear();
	normals.clear();
	for (auto poly : polys)
	{
		delete poly;
	}
	polys.clear();
}

void MeshLoader::load(const char* filename)
{
	char* srcStr = readfile(filename);
	cout << "finish reading file\n";
	if (srcStr != nullptr)
	{
		char* subStr = srcStr;
		int err;
		int offset = 0;
		char trash[255] = {};
		char lineHeader[2] = {};
		float val[3] = {};
		uint32_t indices[3];

		do
		{
			// Skip end of line
			if (*subStr == '\n' || *subStr == '\r')
			{
				subStr++;
				continue;
			}
			lineHeader[0] = lineHeader[1] = 0;
			sscanf(subStr, "%s%n", &lineHeader, &offset);
			subStr += offset;
			// Vertex
			if (strcmp(lineHeader, "v") == 0)
			{
				sscanf(subStr, "%f %f %f%n", val, val + 1, val + 2, &offset);
				//err = sscanf(subStr, "%f %f %f%n", val, val + 1, val + 2, &offset);
				//vertices.push_back(QVector3D(val[0], val[1], val[2]));
				//vertices.insert(vertices.end(), val, val + 3);
				vertices.push_back(val[0]);
				vertices.push_back(val[1]);
				vertices.push_back(val[2]);
				subStr += offset + 1;
			}
			// Texture Coordinate
			else if (strcmp(lineHeader, "vt") == 0)
			{
				//float val[2];
				err = sscanf(subStr, "%f %f%n", val, val + 1, &offset);
				//uvs.push_back(QVector2D(val[0], val[1]));
				//uvs.insert(uvs.end(), val, val + 2);

				uvs.push_back(val[0]);
				uvs.push_back(val[1]);
				subStr += offset + 1;
			}
			// Vertex Normal
			else if (strcmp(lineHeader, "vn") == 0)
			{
				//float val[3];
				err = sscanf(subStr, "%f %f %f%n", val, val + 1, val + 2, &offset);
				//normals.push_back(QVector3D(val[0], val[1], val[2]));
				//normals.insert(normals.end(), val, val + 3);
				normals.push_back(val[0]);
				normals.push_back(val[1]);
				normals.push_back(val[2]);
				subStr += offset + 1;
			}
			// Face Index
			else if (strcmp(lineHeader, "f") == 0)
			{
				cout << "loading face\n";
				PolyIndex* fid = new PolyIndex;
				err = sscanf(subStr, "%s%n", &trash, &offset);
				indices[1] = indices[2] = 0;
				index_t ft = facetype(trash, indices);
				fid->push_back(indices);
				subStr += offset;

				switch (ft)
				{
				case VTN://111
					while (*subStr != '\n' && *subStr != '\r' && *subStr != '\0')
					{
						err = sscanf(subStr, "%s%n", &trash, &offset);
						sscanf(trash, "%d/%d/%d", indices, indices + 1, indices + 2);
						fid->push_back(indices);
						subStr += offset;
					}
					break;
				case VT://011
					while (*subStr != '\n' && *subStr != '\r' && *subStr != '\0')
					{
						err = sscanf(subStr, "%s%n", &trash, &offset);
						sscanf(trash, "%d/%d", indices, indices + 1);
						fid->push_back(indices);
						subStr += offset;
					}
					break;
				case VN://101
					while (*subStr != '\n' && *subStr != '\r' && *subStr != '\0')
					{
						err = sscanf(subStr, "%s%n", &trash, &offset);
						sscanf(trash, "%d//%d", indices, indices + 2);
						fid->push_back(indices);
						subStr += offset;
					}
					break;
				case V://001
					while (*subStr != '\n' && *subStr != '\r' && *subStr != '\0')
					{
						err = sscanf(subStr, "%s%n", &trash, &offset);
						sscanf(trash, "%d", indices);
						fid->push_back(indices);
						subStr += offset;
					}
					break;
				default:
					break;
				}
				fid->size = fid->v.size();
				polys.push_back(fid);
				subStr++;
			}
			// Comment
			else if (strcmp(lineHeader, "#") == 0)
			{
				err = sscanf(subStr, "%[^\r\n]%n", &trash, &offset);
				subStr += offset + 1;
			}
			// Others
			else
			{
				// skip everything except \n
				err = sscanf(subStr, "%[^\r\n]%n", &trash, &offset);
				//cout << lineHeader << trash << "\n";
				subStr += offset + 1;
			}

		} while (*subStr != '\0');

		delete[] srcStr;
	}
}

void MeshLoader::load_from_file(const char * filename)
{
	FILE* fp = fopen(filename, "r");
	if (fp == nullptr)
	{
		return;
	}
	int err;
	char buff[255] = {};
	char lineHeader[2] = {};
	float val[3] = {};
	uint32_t indices[3];
	char endflg;

	while (true)
	{
		lineHeader[0] = lineHeader[1] = 0;
		err = fscanf(fp, "%2s", &lineHeader);
		if (err == EOF)
		{
			break;
		}
		// Vertex
		if (strcmp(lineHeader, "v") == 0)
		{
			fscanf(fp, "%f %f %f\n", val, val + 1, val + 2);
			vertices.insert(vertices.end(), val, val + 3);
			/*vertices.push_back(val[0]);
			vertices.push_back(val[1]);
			vertices.push_back(val[2]);*/
		}
		// Texture Coordinate
		else if (strcmp(lineHeader, "vt") == 0)
		{
			fscanf(fp, "%f %f\n", val, val + 1);
			uvs.insert(uvs.end(), val, val + 2);

			/*uvs.push_back(val[0]);
			uvs.push_back(val[1]);*/
		}
		// Vertex Normal
		else if (strcmp(lineHeader, "vn") == 0)
		{
			//float val[3];
			fscanf(fp, "%f %f %f\n", val, val + 1, val + 2);
			normals.insert(normals.end(), val, val + 3);
			/*normals.push_back(val[0]);
			normals.push_back(val[1]);
			normals.push_back(val[2]);*/
		}
		// Face Index
		else if (strcmp(lineHeader, "f") == 0)
		{
			//cout << "loading face\n";
			PolyIndex* fid = new PolyIndex;
			err = fscanf(fp, "%s", &buff);
			indices[1] = indices[2] = 0;
			index_t ft = facetype(buff, indices);
			fid->push_back(indices);
			endflg = fgetc(fp);
			switch (ft)
			{
			case VTN://111
				while (endflg != '\n' && endflg != '\r' && endflg != '\0')
				{
					ungetc(endflg, fp);
					fscanf(fp, "%d/%d/%d", indices, indices + 1, indices + 2);
					fid->push_back(indices);
					endflg = fgetc(fp);
				}
				break;
			case VT://011
				while (endflg != '\n' && endflg != '\r' && endflg != '\0')
				{
					ungetc(endflg, fp);
					fscanf(fp, "%d/%d", indices, indices + 1);
					fid->push_back(indices);
					endflg = fgetc(fp);
				}
				break;
			case VN://101
				while (endflg != '\n' && endflg != '\r' && endflg != '\0')
				{
					ungetc(endflg, fp);
					fscanf(fp, "%d//%d", indices, indices + 2);
					fid->push_back(indices);
					endflg = fgetc(fp);
				}
				break;
			case V://001
				while (endflg != '\n' && endflg != '\r' && endflg != '\0')
				{
					ungetc(endflg, fp);
					fscanf(fp, "%d", indices);
					fid->push_back(indices);
					endflg = fgetc(fp);
				}
				break;
			default:
				break;
			}
			fid->size = fid->v.size();
			polys.push_back(fid);
		}
		// Comment
		else if (strcmp(lineHeader, "#") == 0)
		{
			fscanf(fp, "%[^\r\n]", &buff);
		}
		// Others
		else
		{
			// skip everything except \n
			fscanf(fp, "%[^\r\n]", &buff);
			//cout << lineHeader << trash << "\n";
		}

	}

	fclose(fp);
}

void MeshLoader::exportVBO(floats_t * verts, floats_t * texcoords, floats_t * norms, ui32s_t * fids) const
{
	if (verts != nullptr)
	{
		verts->clear();
		verts->reserve(vertices.size() * 3);
		/*for (auto v : vertices)
		{
			verts->push_back(v.x());
			verts->push_back(v.y());
			verts->push_back(v.z());
		}*/
		verts->insert(verts->end(), vertices.begin(), vertices.end());
	}
	if (fids != nullptr)
	{
		fids->clear();
		fids->reserve(polys.size() * 3);
		for (auto poly : polys)
		{
			//uint32_t size = poly->size;
			auto vid = poly->v;
			/*if (vid)
			{
			}*/
				for (int i = 1; i < poly->v.size() - 1;)
				{
					fids->push_back(vid[0]);
					fids->push_back(vid[i++]);
					fids->push_back(vid[i]);
				}
		}
	}
}

char* MeshLoader::readfile(const char* filename)
{
#if 0
	clock_t start = clock();
#endif

	FILE* fp = fopen(filename, "r");
	if (fp == nullptr)
	{
		return nullptr;
	}
	fseek(fp, 0L, SEEK_END);
	auto fsize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	char* cstr = new char[fsize + 1]();
#ifdef _MSC_VER
	fread_s(cstr, sizeof(char) * fsize, sizeof(char), fsize, fp);
#else
	fread(cstr, sizeof(char), fsize, fp);
#endif

	fclose(fp);
	return cstr;
}

MeshLoader::index_t MeshLoader::facetype(const char* str, uint32_t* val)
{
	int argv = sscanf(str, "%d/%d/%d", val, val + 1, val + 2);
	switch (argv)
	{
	case 3:// V/T/N
		return VTN;//111
	case 2:// V/T
		return VT;//011
	case 1:
		argv = sscanf(str, "%d//%d", val, val + 2);
		if (argv == 2)// V//N
		{
			return VN;//101
		}
		else// V
		{
			return V;//001
		}
	}
}