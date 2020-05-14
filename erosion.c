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
	int rowByteSize;		// rozmiar wiersza w bajtach
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
	if ((pInfo = (imgInfo *) malloc(sizeof(imgInfo))) == 0)
		return (imgInfo*) freeResources(fbmp, pInfo->pImg, pInfo);

	pInfo->width = bmpHead.biWidth;
	pInfo->height = bmpHead.biHeight;
	imageSize = (((pInfo->width + 31) >> 5) << 2) * pInfo->height;

	if ((pInfo->pImg = (unsigned char*) malloc(imageSize)) == 0)
		return (imgInfo*) freeResources(fbmp, pInfo->pImg, pInfo);

	// process height (it can be negative)
	ptr = pInfo->pImg;
	lineBytes = ((pInfo->width + 31) >> 5) << 2; // line size in bytes
	pInfo->rowByteSize = lineBytes;

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
	memset(pImg->pImg, 0, (((w + 31) >> 5) << 2) * h);
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

/****************************************************************************************/

extern unsigned int TwoWordsErosion(unsigned int word1, unsigned int word2);

extern void MiddleErosion(unsigned char* pImg, unsigned char* pImgCopy, int byteNum1, int byteNum2);

extern void LeftEdgeErosion(unsigned char* pImg, unsigned char* pImgCopy, int byteNum1, int byteNum2);


void RightEdgeErosion(imgInfo* pImg, imgInfo* pImgCopy, int byteNum1, int byteNum2)
{
	unsigned char *pByte1 = pImg->pImg + byteNum1;
	unsigned char *pByte2 = pImg->pImg + byteNum2;
	unsigned char *pByteCopy = pImgCopy->pImg + byteNum1;

	unsigned short twoBytes1, twoBytes2;

	twoBytes1 = (*(pByte1 - 1) << 8) + *pByte1;
	twoBytes2 = (*(pByte2 - 1) << 8) + *pByte2;


	twoBytes1 = TwoWordsErosion(twoBytes1, twoBytes2);

	*pByteCopy = *pByteCopy | twoBytes1;
}


imgInfo* Erosion(imgInfo* pImg)
{
	imgInfo* pErodedImg;
	int byteColumn;

	pErodedImg = InitScreen(pImg->width, pImg->height);

	int byteSize = pImg->height * pImg->rowByteSize;
	int byteWidth = (pImg->width + 7) >> 3;

	
	for (int i = 0; i < byteSize; i++)
	{
		byteColumn = i % pImg->rowByteSize;
		
		// upper edge
		if (i < pImg->rowByteSize)
		{
			// left corner
			if (byteColumn == 0)
			{
				LeftEdgeErosion(pImg->pImg, pErodedImg->pImg, i, i);
				LeftEdgeErosion(pImg->pImg, pErodedImg->pImg, i, i + pImg->rowByteSize);
			}
			// right corner
			else if (byteColumn == (byteWidth - 1))
			{
				RightEdgeErosion(pImg, pErodedImg, i, i);
				RightEdgeErosion(pImg, pErodedImg, i, i + pImg->rowByteSize);
			}
			// middle
			else
			{
				MiddleErosion(pImg->pImg, pErodedImg->pImg, i, i);
				MiddleErosion(pImg->pImg, pErodedImg->pImg, i, i + pImg->rowByteSize);
			}
		}
		// bottom edge
		else if (i > (byteSize - pImg->rowByteSize))
		{
			// left corner
			if (byteColumn == 0)
			{
				LeftEdgeErosion(pImg->pImg, pErodedImg->pImg, i, i);
				LeftEdgeErosion(pImg->pImg, pErodedImg->pImg, i, i - pImg->rowByteSize);
			}
			// right corner
			else if (byteColumn == (byteWidth - 1))
			{
				RightEdgeErosion(pImg, pErodedImg, i, i);
				RightEdgeErosion(pImg, pErodedImg, i, i - pImg->rowByteSize);
			}
			// middle
			else
			{
				MiddleErosion(pImg->pImg, pErodedImg->pImg, i, i);
				MiddleErosion(pImg->pImg, pErodedImg->pImg, i, i - pImg->rowByteSize);
			}
		}
		// middle
		else
		{
			// left corner
			if (byteColumn == 0)
			{
				LeftEdgeErosion(pImg->pImg, pErodedImg->pImg, i, i);
				LeftEdgeErosion(pImg->pImg, pErodedImg->pImg, i, i + pImg->rowByteSize);
				LeftEdgeErosion(pImg->pImg, pErodedImg->pImg, i, i - pImg->rowByteSize);
			}
			// right corner
			else if (byteColumn == (byteWidth - 1))
			{
				RightEdgeErosion(pImg, pErodedImg, i, i);
				RightEdgeErosion(pImg, pErodedImg, i, i + pImg->rowByteSize);
				RightEdgeErosion(pImg, pErodedImg, i, i - pImg->rowByteSize);
			}
			// middle
			else
			{
				MiddleErosion(pImg->pImg, pErodedImg->pImg, i, i);
				MiddleErosion(pImg->pImg, pErodedImg->pImg, i, i + pImg->rowByteSize);
				MiddleErosion(pImg->pImg, pErodedImg->pImg, i, i - pImg->rowByteSize);
			}
		}
	}

	return pErodedImg;
}

/****************************************************************************************/

int main(int argc, char* argv[])
{
	imgInfo* pInfo;
	imgInfo* pErodedImg;
	int i;

	printf("Size of bmpHeader = %d\n", sizeof(bmpHdr));
	
	if (sizeof(bmpHdr) != 62)
	{
		printf("Change compilation options so as bmpHdr struct size is 62 bytes.\n");
		return 1;
	}


	pInfo = readBMP("test4.bmp");
	/*
	printf("poczatek pliku: %#x\n", pInfo->pImg);
	printf("aktualne wpolrzedne: (%d, %d)\n", pInfo->cX, pInfo->cY);
	printf("aktualny kolor: %d\n", pInfo->col);
	printf("wysokosc: %d\n", pInfo->height);
	printf("szerokosc: %d\n", pInfo->width);
	*/
	pErodedImg = Erosion(pInfo);

	saveBMP(pErodedImg, "result.bmp");

	FreeScreen(pErodedImg);
	FreeScreen(pInfo);
	
	return 0;
}
