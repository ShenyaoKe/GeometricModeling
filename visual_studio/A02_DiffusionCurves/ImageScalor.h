#pragma once
#ifndef __IMAGESCALOR__
#define __IMAGESCALOR__
//#include "Image/ImageData.h"
#include <QImage>

QImage* downscale(const QImage* origin);

// Image Pyramid like structure
// Bottom image has bigger size than top image
QImage* upscale(const QImage* btmImg, const QImage* topImg);

void smooth(QImage* image);
/*
ImageData ImageScalor(const ImageData& origin)
{
	int w = origin.getWidth();
	int h = origin.getHeight();
	ImageData ret(w >> 1, h >> 1);
	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			ColorRGBA newC(0,0,0,0);
			int count = 0;
			for (int m = 0; m < 2; m++)
			{
				for (int n = 0; n < 2; n++)
				{
					auto curC = origin.getRGBA(2 * i + m, 2 * j + n);
					if (curC.a = 1.0)
					{
						newC += curC;
						count++;
					}
				}
			}
			if (count > 0)
			{
				newC /= static_cast<Float>(count);
				newC.a = 1.0;
			}
			ret.setRGBA(i, j, newC);
		}
	}
	return ret;
}*/
#endif