#include "MeshLoader.h"

MeshLoader::MeshLoader(const char* filename)
{
	char* srcStr = readfile(filename);
	//char* cstr = "#1111111\n";
	if (srcStr != nullptr)
	{
		char* subStr = srcStr;
		int err;
		int offset = 0;
		int d[3] = {};
		char trash[255] = {};
		char lineHeader[2] = {};

		do
		{
			if (*subStr == '\n')
			{
				subStr++;
				continue;
			}
			lineHeader[0] = lineHeader[1] = 0;
			sscanf(subStr, "%s%n", &lineHeader, &offset);
			subStr += offset;
			if (strcmp(lineHeader, "v") == 0)
			{
				float val[3];
				err = sscanf(subStr, "%f %f %f%n", val, val + 1, val + 2, &offset);
				vertices.push_back(QVector3D(val[0], val[1], val[2]));
				subStr += offset + 1;
			}
			else if (strcmp(lineHeader, "vt") == 0)
			{
				float val[2];
				err = sscanf(subStr, "%f %f%n", val, val + 1, &offset);
				uvs.push_back(QVector2D(val[0], val[1]));

				subStr += offset + 1;
			}
			else if (strcmp(lineHeader, "vn") == 0)
			{
				float val[3];
				err = sscanf(subStr, "%f %f %f%n", val, val + 1, val + 2, &offset);
				normals.push_back(QVector3D(val[0], val[1], val[2]));
				subStr += offset + 1;
			}
			else if (strcmp(lineHeader, "f") == 0)
			{
				PolyIndex* fid = new PolyIndex;
				err = sscanf(subStr, "%s%n", &trash, &offset);
				int indices[3] = {};
				index_t ft = facetype(trash, indices);
				fid->push_back(indices);
				subStr += offset;


				switch (ft)
				{
				case VTN://111
					while (*subStr != '\n' && *subStr != '\0')
					{
						err = sscanf(subStr, "%s%n", &trash, &offset);
						sscanf(trash, "%d/%d/%d", indices, indices + 1, indices + 2);
						subStr += offset;
					}
					break;
				case VT://011
					while (*subStr != '\n' && *subStr != '\0')
					{
						err = sscanf(subStr, "%s%n", &trash, &offset);
						sscanf(trash, "%d/%d", indices, indices + 1);
						subStr += offset;
					}
					break;
				case VN://101
					while (*subStr != '\n' && *subStr != '\0')
					{
						err = sscanf(subStr, "%s%n", &trash, &offset);
						sscanf(trash, "%d//%d", indices, indices + 2);
						subStr += offset;
					}
					break;
				case V://001
					while (*subStr != '\n' && *subStr != '\0')
					{
						err = sscanf(subStr, "%s%n", &trash, &offset);
						sscanf(trash, "%d", indices);
						subStr += offset;
					}
					break;
				default:
					break;
				}
				polys.push_back(fid);
				subStr++;
			}
			else if (strcmp(lineHeader, "#") == 0)
			{
				err = sscanf(subStr, "%[^\n]%n", &trash, &offset);
				//cout << "#" << trash << "\n";
				subStr += offset + 1;
			}
			else
			{
				// skip everything except \n
				err = sscanf(subStr, "%[^\n]%n", &trash, &offset);
				//cout << lineHeader << trash << "\n";
				subStr += offset + 1;
			}

		} while (*subStr != '\0');
		delete[] srcStr;
	}
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
	//fread_s(cstr, sizeof(char) * fsize, sizeof(char), fsize, fp);
	fread(cstr, sizeof(char), fsize, fp);
#if 0
	clock_t endt = clock();
	cout << "byte size" << fsize
		<< "\ntime dur:" << endt - start << endl;

	for (int i = 0; i <= fsize; i++)
	{
		cout << i << ":\t" << (int)cstr[i] << endl;
	}
#endif

	fclose(fp);
	return cstr;
}



MeshLoader::index_t MeshLoader::facetype(const char* str, int* val)
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