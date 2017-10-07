#include "bitmap.h"
#include <fstream>
#include <iostream>

using namespace std;

#ifndef WIN32
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

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
#else
#include <windows.h>
#endif


bool Bitmap::openFromFile(const char *fileName)
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
#define READMEMBER(member) \
	file.read((char *)&member, sizeof(member));


	// this spells trouble, different compilers will align members and structs differently. seekg might help if members are quasi aligned the same
	READMEMBER(fileHeader.bfType)
	READMEMBER(fileHeader.bfSize)
	READMEMBER(fileHeader.bfReserved1)
	READMEMBER(fileHeader.bfReserved2)
	READMEMBER(fileHeader.bfOffBits)

	READMEMBER(infoHeader.biSize)
	READMEMBER(infoHeader.biWidth)
	READMEMBER(infoHeader.biHeight)
	READMEMBER(infoHeader.biPlanes)
	READMEMBER(infoHeader.biBitCount)
	READMEMBER(infoHeader.biCompression)
	READMEMBER(infoHeader.biSizeImage)
	READMEMBER(infoHeader.biXPelsPerMeter)
	READMEMBER(infoHeader.biYPelsPerMeter)
	READMEMBER(infoHeader.biClrUsed)
	READMEMBER(infoHeader.biClrImportant)

	m_width = infoHeader.biWidth;
	m_height = infoHeader.biHeight;
	m_length = m_width * m_height * 3;

	if(m_length == 0)
	{
		cout << "Empty image. Length read: " << m_length << endl;
		m_width = 0;
		m_height = 0;
		return false;
	}

	m_data.reset(new uint8_t[m_length]);

	if (m_data.get() == nullptr)
	{
		cout << "Error during allocation: " << m_length << endl;
		m_width = 0;
		m_height = 0;
		return false;
	}

	//fill the bitmap in the memory
	file.read(reinterpret_cast<char*> (m_data.get()), m_length);

	file.close();
	return true;
}
