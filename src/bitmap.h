#pragma once

class Bitmap
{
	public:
		Bitmap() 
			:data(0)
			, format(0)
			, length(0)
			, width(0)
			, height(0)
		{}
		~Bitmap() { delete [] data; }
		bool CreateFromFile(const char *);
		const char *getData() {return data;}
		int getFormat() { return format; }
		int getLength() { return length; }
		int getWidth() { return width; }
		int getHeight() { return height; }

	private:
		char *data;
		int format;
		long length;
		int width;
		int height;
};
