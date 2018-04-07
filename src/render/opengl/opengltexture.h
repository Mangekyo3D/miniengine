#pragma once
#include "../itexture.h"

class COpenGLTexture : public ITexture
{
	public:
		COpenGLTexture(EFormat format, uint16_t width, uint16_t height, bool bMipmapped);
		~COpenGLTexture();
		void bind(uint8_t unit);
		uint32_t getID() { return m_id; }

	protected:
		uint32_t formatToGLFormat();
		uint32_t m_id;
};
