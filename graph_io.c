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

void SetPixel(imgInfo* pImg, int x, int y)
{
	unsigned char *pPix = pImg->pImg + (((pImg->width + 31) >> 5) << 2) * y + (x >> 3);
	unsigned char mask = 0x80 >> (x & 0x07);
	if (pImg->col)
		*pPix |= mask;
	else
		*pPix &= ~mask;
}

/****************************************************************************************/

void TwoBytesErosion(imgInfo* pImg, imgInfo* pImgCopy, int byteNum1, int byteNum2)
{
	unsigned char *pByte1 = pImg->pImg + byteNum1;
	unsigned char *pByte2 = pImg->pImg + byteNum2;
	unsigned char *pByteCopy = pImgCopy->pImg + byteNum1;

	unsigned int threeBytes1, threeBytes2;

	threeBytes1 = (*(pByte1 - 1) << 16) + (*pByte1 << 8) + (*(pByte1 + 1));
	threeBytes2 = (*(pByte2 - 1) << 16) + (*pByte2 << 8) + (*(pByte2 + 1));


	threeBytes1 = threeBytes1 | threeBytes2;
	threeBytes1 = threeBytes1 | (threeBytes2 >> 1);
	threeBytes1 = threeBytes1 | (threeBytes2 << 1);

	*pByteCopy = *pByteCopy | (threeBytes1 >> 8);
}


imgInfo* Erosion(imgInfo* pImg)
{
	imgInfo* pErosionInfo;

	pErosionInfo = InitScreen(pImg->width, pImg->height);

	int byteSize = pImg->height * pImg->rowByteSize;
	int byteWidth = (pImg->width + 7) >> 3;

	for (int i = 0; i < byteSize; i++)
	{
		if (i < pImg->rowByteSize)
		{
			TwoBytesErosion(pImg, pErosionInfo, i, i);
			TwoBytesErosion(pImg, pErosionInfo, i, i + pImg->rowByteSize);
		}
		else if (i > (byteSize - pImg->rowByteSize))
		{
			TwoBytesErosion(pImg, pErosionInfo, i, i);
			TwoBytesErosion(pImg, pErosionInfo, i, i - pImg->rowByteSize);
		}
		else
		{
			TwoBytesErosion(pImg, pErosionInfo, i, i);
			TwoBytesErosion(pImg, pErosionInfo, i, i + pImg->rowByteSize);
			TwoBytesErosion(pImg, pErosionInfo, i, i - pImg->rowByteSize);
		}
	}

	return pErosionInfo;
}

/****************************************************************************************/

int main(int argc, char* argv[])
{
	imgInfo* pInfo;
	imgInfo* pErosionInfo;
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
	pErosionInfo = Erosion(pInfo);

	saveBMP(pErosionInfo, "result2.bmp");

	FreeScreen(pErosionInfo);
	FreeScreen(pInfo);
	
	return 0;
}

