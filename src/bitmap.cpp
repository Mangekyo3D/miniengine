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
#include <Windows.h>
#endif


bool BmpReader::openFromFile(const char *fileName, bool bOnlySize)
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	ifstream file(fileName, ios::in|ios::binary);

	if(!file)
	{
		cout << fileName << " not found!" << endl;
		return false;
	}
#define READMEMBER(member) \
	file.read(reinterpret_cast<char *>(&member), sizeof(member));


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

	m_width = static_cast<uint32_t> (infoHeader.biWidth);
	m_height = static_cast<uint32_t> (infoHeader.biHeight);

	size_t padding = (m_width * 3) % 4;
	size_t length = m_width * m_height * 3;

	if (bOnlySize)
	{
		return true;
	}

	if(length == 0)
	{
		cout << "Empty image. Length read: " << length << endl;
		m_width = 0;
		m_height = 0;
		return false;
	}

	m_data.reset(new uint8_t[length]);

	if (m_data.get() == nullptr)
	{
		cout << "Error during allocation: " << length << endl;
		m_width = 0;
		m_height = 0;
		return false;
	}

	//fill the bitmap in the memory
	if (padding == 0)
	{
		file.read(reinterpret_cast<char*> (m_data.get()), static_cast<std::streamsize> (length));
	}
	else
	{
		padding = 4 - padding;

		size_t offset = 0;
		for (uint32_t i = 0; i < m_height; ++i)
		{
			file.read(reinterpret_cast<char*> (m_data.get() + offset), m_width * 3);
			offset += m_width * 3;
			// push the reader forward by reading into a bogus variable. Width needs to be padded to 4 bytes
			// so reading into an int should be OK
			uint32_t stub;
			file.read(reinterpret_cast<char*> (&stub), static_cast<std::streamsize> (padding));
		}
	}

	return true;
}
