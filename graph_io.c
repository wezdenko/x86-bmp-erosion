// graph_io.c : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

// kompilowanie programu wymaga nastepujacych opcji:
// -lm  ze wzgledu na sinus i cosinus
// -m32 bo projekt powinien byc 32-bitowy (int = 4 bajty)
// -fpack-struct  bo header (obrazu cz-bialego) ma miec 62 bajty

// gcc -lm -m32 -fpack-struct graph_io.c

typedef struct
{
	unsigned short bfType; 
	unsigned long  bfSize; 
	unsigned short bfReserved1; 
	unsigned short bfReserved2; 
	unsigned long  bfOffBits; 
	unsigned long  biSize; 
	long  biWidth; 
	long  biHeight; 
	short biPlanes; 
	short biBitCount; 
	unsigned long  biCompression; 
	unsigned long  biSizeImage; 
	long biXPelsPerMeter; 
	long biYPelsPerMeter; 
	unsigned long  biClrUsed; 
	unsigned long  biClrImportant;
	unsigned long  RGBQuad_0;
	unsigned long  RGBQuad_1;
} bmpHdr;

typedef struct
{
	int width, height;		// szerokosc i wysokosc obrazu
	unsigned char* pImg;	// wskazanie na początek danych pikselowych
	int cX, cY;				// "aktualne współrzędne" 
	int col;				// "aktualny kolor"
} imgInfo;

void* freeResources(FILE* pFile, void* pFirst, void* pSnd)
{
	if (pFile != 0)
		fclose(pFile);
	if (pFirst != 0)
		free(pFirst);
	if (pSnd !=0)
		free(pSnd);
	return 0;
}

imgInfo* readBMP(const char* fname)
{
	imgInfo* pInfo = 0;
	FILE* fbmp = 0;
	bmpHdr bmpHead;
	int lineBytes, y;
	unsigned long imageSize = 0;
	unsigned char* ptr;

	pInfo = 0;
	fbmp = fopen(fname, "rb");
	if (fbmp == 0)
		return 0;

	fread((void *) &bmpHead, sizeof(bmpHead), 1, fbmp);
	// some basic checks
	if (bmpHead.bfType != 0x4D42 || bmpHead.biPlanes != 1 ||
		bmpHead.biBitCount != 1 || bmpHead.biClrUsed != 2 ||
		(pInfo = (imgInfo *) malloc(sizeof(imgInfo))) == 0)
		return (imgInfo*) freeResources(fbmp, pInfo->pImg, pInfo);

	pInfo->width = bmpHead.biWidth;
	pInfo->height = bmpHead.biHeight;
	imageSize = (((pInfo->width + 31) >> 5) << 2) * pInfo->height;

	if ((pInfo->pImg = (unsigned char*) malloc(imageSize)) == 0)
		return (imgInfo*) freeResources(fbmp, pInfo->pImg, pInfo);

	// process height (it can be negative)
	ptr = pInfo->pImg;
	lineBytes = ((pInfo->width + 31) >> 5) << 2; // line size in bytes
	if (pInfo->height > 0)
	{
		// "upside down", bottom of the image first
		ptr += lineBytes * (pInfo->height - 1);
		lineBytes = -lineBytes;
	}
	else
		pInfo->height = -pInfo->height;

	// reading image
	// moving to the proper position in the file
	if (fseek(fbmp, bmpHead.bfOffBits, SEEK_SET) != 0)
		return (imgInfo*) freeResources(fbmp, pInfo->pImg, pInfo);

	for (y=0; y<pInfo->height; ++y)
	{
		fread(ptr, 1, abs(lineBytes), fbmp);
		ptr += lineBytes;
	}
	fclose(fbmp);
	return pInfo;
}

int saveBMP(const imgInfo* pInfo, const char* fname)
{
	int lineBytes = ((pInfo->width + 31) >> 5)<<2;
	bmpHdr bmpHead = 
	{
	0x4D42,				// unsigned short bfType; 
	sizeof(bmpHdr),		// unsigned long  bfSize; 
	0, 0,				// unsigned short bfReserved1, bfReserved2; 
	sizeof(bmpHdr),		// unsigned long  bfOffBits; 
	40,					// unsigned long  biSize; 
	pInfo->width,		// long  biWidth; 
	pInfo->height,		// long  biHeight; 
	1,					// short biPlanes; 
	1,					// short biBitCount; 
	0,					// unsigned long  biCompression; 
	lineBytes * pInfo->height,	// unsigned long  biSizeImage; 
	11811,				// long biXPelsPerMeter; = 300 dpi
	11811,				// long biYPelsPerMeter; 
	2,					// unsigned long  biClrUsed; 
	0,					// unsigned long  biClrImportant;
	0x00000000,			// unsigned long  RGBQuad_0;
	0x00FFFFFF			// unsigned long  RGBQuad_1;
	};

	FILE * fbmp;
	unsigned char *ptr;
	int y;

	if ((fbmp = fopen(fname, "wb")) == 0)
		return -1;
	if (fwrite(&bmpHead, sizeof(bmpHdr), 1, fbmp) != 1)
	{
		fclose(fbmp);
		return -2;
	}

	ptr = pInfo->pImg + lineBytes * (pInfo->height - 1);
	for (y=pInfo->height; y > 0; --y, ptr -= lineBytes)
		if (fwrite(ptr, sizeof(unsigned char), lineBytes, fbmp) != lineBytes)
		{
			fclose(fbmp);
			return -3;
		}
	fclose(fbmp);
	return 0;
}

/****************************************************************************************/
imgInfo* InitScreen (int w, int h)
{
	imgInfo *pImg;
	if ( (pImg = (imgInfo *) malloc(sizeof(imgInfo))) == 0)
		return 0;
	pImg->height = h;
	pImg->width = w;
	pImg->pImg = (unsigned char*) malloc((((w + 31) >> 5) << 2) * h);
	if (pImg->pImg == 0)
	{
		free(pImg);
		return 0;
	}
	memset(pImg->pImg, 0xFF, (((w + 31) >> 5) << 2) * h);
	pImg->cX = 0;
	pImg->cY = 0;
	pImg->col = 0;
	return pImg;
}

void FreeScreen(imgInfo* pInfo)
{
	if (pInfo && pInfo->pImg)
		free(pInfo->pImg);
	if (pInfo)
		free(pInfo);
}

imgInfo* SetColor(imgInfo* pImg, int col)
{
	pImg->col = col != 0;
	return pImg;
}

imgInfo* MoveTo(imgInfo* pImg, int x, int y)
{
	if (x >= 0 && x < pImg->width)
		pImg->cX = x;
	if (y >= 0 && y < pImg->height)
		pImg->cY = y;
	return pImg;
}

void SetPixel(imgInfo* pImg, int x, int y)
{
	unsigned char *pPix = pImg->pImg + (((pImg->width + 31) >> 5) << 2) * y + (x >> 3);
	unsigned char mask = 0x80 >> (x & 0x07);
	if (pImg->col)
		*pPix |= mask;
	else
		*pPix &= ~mask;
}

imgInfo* LineTo(imgInfo* pImg, int x, int y)
{
	// draws line segment between current position and (x,y)
	int cx = pImg->cX, cy = pImg->cY;
	int dx = x - cx, xi = 1, dy = y - cy, yi = 1;
	int d, ai, bi;

	if (dx < 0)
	{ 
		xi = -1;
		dx = -dx;
	} 

	if (dy < 0)
	{ 
		yi = -1;
		dy = -dy;
	} 

	// first pixel
	SetPixel(pImg, cx, cy);

	// horizontal drawing 
	if (dx > dy)
	{
		ai = (dy - dx) * 2;
		bi = dy * 2;
		d = bi - dx;
		// for each x
		while (cx != x)
		{ 
			// check line move indicator
			if (d >= 0)
			{ 
				cx += xi;
				cy += yi;
				d += ai;
			} 
			else
			{
				d += bi;
				cx += xi;
			}
			SetPixel(pImg, cx, cy);
		}
	} 
	// vertical drawing
	else
	{ 
		ai = ( dx - dy ) * 2;
		bi = dx * 2;
		d = bi - dy;
		// for each y
		while (cy != y)
		{ 
			// check column move indicator
			if (d >= 0)
			{ 
				cx += xi;
				cy += yi;
				d += ai;
			}
			else
			{
				d += bi;
				cy += yi;
			}
			SetPixel(pImg, cx, cy);
		}
	}
	pImg->cX = x;
	pImg->cY = y;
	return pImg;
}

/****************************************************************************************/

void DrawPolyCircle(imgInfo* pInfo, int vCnt, int nStep, int cX, int cY, int radius)
{
	double pi = 2*asin(1);
	int i;

	MoveTo(pInfo, cX, cY - radius);
	for (i=nStep; i <= nStep * vCnt; i += nStep)
	{
		LineTo(pInfo, 
			(int)(cX + 0.5 + sin((2 * pi * i)/vCnt) * radius), 
			(int)(cY + 0.5 - cos((2 * pi * i)/vCnt) * radius));
	}
}

int main(int argc, char* argv[])
{
	imgInfo* pInfo;
	int i;

	printf("Size of bmpHeader = %d\n", sizeof(bmpHdr));
	if (sizeof(bmpHdr) != 62)
	{
		printf("Change compilation options so as bmpHdr struct size is 62 bytes.\n");
		return 1;
	}
	if ((pInfo = InitScreen (512, 512)) == 0)
		return 2;
	/*
	saveBMP(pInfo, "blank.bmp");
	pInfo = readBMP("blank.bmp");
	*/

	SetColor(pInfo, 0);
	for (i=0; i<256; ++i)
	{
		MoveTo(pInfo, 256, i);
		LineTo(pInfo, 511, i);
		MoveTo(pInfo, i, 256);
		LineTo(pInfo, i, 511);
	}

	DrawPolyCircle(pInfo, 8, 3, 127, 127, 100);
	DrawPolyCircle(pInfo, 16, 5, 383, 383, 100);

	SetColor(pInfo, 1);
	DrawPolyCircle(pInfo, 10, 3, 383, 127, 100);
	DrawPolyCircle(pInfo, 13, 4, 127, 383, 100);

	saveBMP(pInfo, "result.bmp");
	FreeScreen(pInfo);
	return 0;
}

