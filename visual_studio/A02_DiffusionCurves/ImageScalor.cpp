#include "ImageScalor.h"


/*
ImageScalor::ImageScalor()
{
}


ImageScalor::~ImageScalor()
{
}
*/

QImage* downscale(const QImage* origin)
{
	int oriW = origin->width();
	int oriH = origin->height();
	int w = (oriW + 1) >> 1;
	int h = (oriH + 1) >> 1;
	QImage* ret = new QImage(w, h, QImage::Format_ARGB32);
	ret->fill(0);
	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			int r = 0, g = 0, b = 0, a = 0;
			for (int m = -1; m < 2; m++)
			{
				for (int n = -1; n < 2; n++)
				{
					int oriX = 2 * i + m;
					int oriY = 2 * j + n;
					if (oriX >= oriW || oriY >= oriH || oriX < 0 || oriY < 0)
					{
						continue;
					}
					QRgb curC = origin->pixel(oriX, oriY);

					if (qAlpha(curC) == 0xFF)
					{
						r += qRed(curC);
						g += qGreen(curC);
						b += qBlue(curC);
						a++;
					}
				}
			}
			if (a > 0)
			{
				r /= a;
				g /= a;
				b /= a;
				a = 0xFF;
			}
			else
			{
				a = 0;
			}
			ret->setPixel(i, j, qRgba(r, g, b, a));
		}
	}
	return ret;
}
QImage* upscale(const QImage* btmImg, const QImage* topImg)
{
	int w = btmImg->width();
	int h = btmImg->height();
	QImage* ret = new QImage(*btmImg);

	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			QRgb curC = ret->pixel(i, j);
			if (qAlpha(curC) == 0)
			{
				//int topI = (i + 1) >> 1, topJ = (j + 1) >> 1;
				int topI = i / 2, topJ = j / 2;
				QRgb val = topImg->pixel(topI, topJ);
				ret->setPixel(i, j, val);
			}
		}
	}

	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			QRgb curC = btmImg->pixel(i, j);
			if (qAlpha(curC) == 0)
			{
				int r = 0, g = 0, b = 0, count = 0;
				for (int m = -1; m < 2; m++)
				{
					for (int n = -1; n < 2; n++)
					{
						int oriX = i + m;
						int oriY = j + n;
						if (oriX >= w || oriY >= h || oriX < 0 || oriY < 0)
						{
							continue;
						}
						QRgb sampleC = ret->pixel(oriX, oriY);

						r += qRed(sampleC);
						g += qGreen(sampleC);
						b += qBlue(sampleC);
						count++;
					}
				}
				if (count > 0)
				{
					r /= count;
					g /= count;
					b /= count;
				}
				ret->setPixel(i, j, qRgba(r, g, b, 0xFF));
			}
		}
	}


	return ret;
}

void smooth(QImage* image)
{

}
