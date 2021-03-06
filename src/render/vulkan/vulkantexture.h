#pragma once
#include "../itexture.h"
#include "vulkan/vulkan.h"

struct SMemoryChunk;

class CVulkanTexture : public ITexture
{
public:
	CVulkanTexture(EFormat type, uint32_t usage, uint32_t width, uint32_t height, bool bMipmapped);
	~CVulkanTexture() override;

	VkFormat typeToFormat();
	VkImageUsageFlags usageFlags();
	operator VkImage () { return m_image; }
	virtual size_t getFormatPixelSize() override;
	VkDescriptorImageInfo getDescriptorImageInfo();

	VkImageView getView() { return m_view; }

private:
	SMemoryChunk*    m_memoryChunk;
	size_t           m_offset;

	VkImage        m_image;
	VkImageView    m_view;
};
