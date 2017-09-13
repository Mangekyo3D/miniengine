#include "GameWindow.h"
#include "../glcorearb.h"
#include <cstring>
#include <fstream>

GameWindow::GameWindow()
{
}

GameWindow::~GameWindow()
{
}

bool GameWindow::queryExtension(const char *extName, const char *extensionList)
{
	/*
	const char *iter = extensionList;

    int nameLength = strlen(extName);
    if(iter == NULL)
        return GL_FALSE;

    while(true)
    {
		const char *internalIter = iter;

        while((*internalIter != ' ') && (*internalIter != '\0'))
            internalIter++;

        if((internalIter-iter) == nameLength)
        {
            int i;
            for(i = 0; *(iter + i) == *(extName+i); i++)
                ;

            if(i==nameLength)
                return GL_TRUE;
        }

        //go past the space
        if(*internalIter == '\0')
			break;
        else iter = internalIter+1;
    }

    return GL_FALSE;
    */

	//rewrite for gl 4.1
	return GL_FALSE;
}

/*
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


void GameWindow::saveGLScreenShot()
{
	std::ofstream file;
	int width;
	int height;
	char *buffer;

	file.open("screenshot.bmp", std::ios::out|std::ios::binary);

	width = getWidth();
	height = getHeight();

	buffer = new char[height*width*3];
	glReadPixels(0 ,0, width, height, GL_BGR, GL_UNSIGNED_BYTE, buffer);

	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	fileHeader.bfType = 0x4D42;
	fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
		+ width*height*3;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits = 54;

	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth = width;
	infoHeader.biHeight = height;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 24;
	infoHeader.biCompression = 0;
	infoHeader.biSizeImage = 0;
	infoHeader.biXPelsPerMeter =
		0;
	infoHeader.biYPelsPerMeter =
		0;
	infoHeader.biClrUsed = 0;
	infoHeader.biClrImportant = 0;

	file.write((char *)&fileHeader, sizeof(BITMAPFILEHEADER));
	file.write((char *)&infoHeader, sizeof(BITMAPINFOHEADER));
	file.write(buffer, 3*width*height);

	delete buffer;
}
*/
