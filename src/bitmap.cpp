#include "bitmap.h"
#include <fstream>
#include "glcorearb.h"
#include <iostream>

using namespace std;

#ifndef WIN32
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;

typedef struct tagBITMAPFILEHEADER {
	WORD	bfType;
	DWORD	bfSize;
	WORD	bfReserved1;
	WORD	bfReserved2;
	DWORD	bfOffBits;
} BITMAPFILEHEADER,*LPBITMAPFILEHEADER,*PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	DWORD	biSize;
	LONG	biWidth;
	LONG	biHeight;
	WORD	biPlanes;
	WORD	biBitCount;
	DWORD	biCompression;
	DWORD	biSizeImage;
	LONG	biXPelsPerMeter;
	LONG	biYPelsPerMeter;
	DWORD	biClrUsed;
	DWORD	biClrImportant;
} BITMAPINFOHEADER,*LPBITMAPINFOHEADER,*PBITMAPINFOHEADER;
#endif


bool Bitmap::CreateFromFile(const char *fileName)
{
	ifstream file;
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	file.open(fileName, ios::in|ios::binary);
	if(!file)
	{
		cout << fileName << " not found!" << endl;
		return false;
	}
	file.read((char *)&fileHeader, sizeof(BITMAPFILEHEADER));
//	file.seekg(14);
	file.read((char *)&infoHeader, sizeof(BITMAPINFOHEADER));

	cout << infoHeader.biSize << endl;
	cout << infoHeader.biWidth << endl;
	cout << infoHeader.biHeight << endl;


	width = infoHeader.biWidth;
	height = infoHeader.biHeight;
	length = width * height * 3;

	if((length == 0) || !(data = new char[length]))
	{
		cout << "Empty image. Length read: " <<length << endl;
		width = 0;
		height = 0;
		return false;
	}

	//fill the bitmap in the memory
	file.read(data, length);

	file.close();
	format = GL_BGR;
	return true;
}
