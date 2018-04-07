#pragma once
#include "vulkan/vulkan.h"

struct SMemoryChunk;

class CVulkanTexture
{
public:
	enum class EType
	{
		eColor,
		eDepth
	};

	enum EUsage
	{
		eAttachement = (1),
		eSampled     = (1 << 1)
	};

	CVulkanTexture(EType type, EUsage usage, uint32_t width, uint32_t height);
	~CVulkanTexture();

	VkFormat typeToFormat();
	VkImageUsageFlags usageFlags();

	VkImageView getView() { return m_view; }

private:
	SMemoryChunk*    m_memoryChunk;
	size_t           m_offset;

	VkImage        m_image;
	VkImageView    m_view;

	EType m_type;
	EUsage m_usage;

	uint32_t m_width;
	uint32_t m_height;
};
