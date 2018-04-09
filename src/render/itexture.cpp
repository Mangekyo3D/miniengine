#include "itexture.h"
#include "../bitmap.h"
#include "../Util/colorutils.h"
#include <algorithm>


ITexture::ITexture(EFormat format, uint32_t usage, uint32_t width, uint32_t height, bool bMipmapped)
	: m_width(width)
	, m_height(height)
	, m_format(format)
	, m_usage(usage)
{
	if (bMipmapped)
	{
		uint16_t maxDim = std::max(m_width, m_height);

		m_mipLevels = 0;

		while (maxDim > 0)
		{
			maxDim >>= 1;
			++m_mipLevels;
		}
	}
	else
	{
		m_mipLevels = 1;
	}
}

bool ITexture::isFormatDepth() const
{
	return (m_format == EFormat::eDepth32f);
}
