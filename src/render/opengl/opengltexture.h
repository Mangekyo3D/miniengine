#pragma once
#include "../itexture.h"

class COpenGLTexture : public ITexture
{
	public:
		COpenGLTexture(EFormat format, uint32_t usage, uint16_t width, uint16_t height, bool bMipmapped);
		~COpenGLTexture() override;
		void bind(uint32_t unit);
		uint32_t getID() { return m_id; }
		virtual size_t getFormatPixelSize() override;

	protected:
		uint32_t formatToGLFormat();
		uint32_t m_id;
};
