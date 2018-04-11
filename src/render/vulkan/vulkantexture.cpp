#include "vulkantexture.h"
#include "vulkandevice.h"
#include <iostream>

CVulkanTexture::CVulkanTexture(EFormat format, uint32_t usage, uint32_t width, uint32_t height, bool bMipmapped)
	: ITexture(format, usage, width, height, bMipmapped)
{
	VkExtent3D extent = {width, height, 1};

	auto& device = CVulkanDevice::get();

	// first ask vulkan if the format we are feeding it is supported
	VkImageCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		nullptr,
		0,
		VK_IMAGE_TYPE_2D,
		typeToFormat(),
		extent,
		m_mipLevels,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		usageFlags(),
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		VK_IMAGE_LAYOUT_UNDEFINED
	};

	// if image is supported, go ahead and create it
	if (device.getSupportsImage(createInfo.format, createInfo.imageType, createInfo.tiling, createInfo.usage, createInfo.flags))
	{
		if (device.vkCreateImage(device, &createInfo, nullptr, &m_image) != VK_SUCCESS)
		{
			std::cout << "Error during image creation" << std::endl;
		}
	}

	if (m_image)
	{
		VkMemoryRequirements memoryRequirements;
		device.vkGetImageMemoryRequirements(device, m_image, &memoryRequirements);

		if (!device.allocateMemory(&m_memoryChunk, m_offset, memoryRequirements, false))
		{
			std::cout << "Could not allocate texture memory" << std::endl;
		}

		device.vkBindImageMemory(device, m_image, m_memoryChunk->m_memory, m_offset);

		// finally, create a view for this image
		VkImageSubresourceRange subresourceRange = {
			static_cast<VkImageAspectFlags> (!isFormatDepth() ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT),
			0,
			1,
			0,
			1
		};

		VkComponentMapping componentMapping = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};

		VkImageViewCreateInfo imageviewcreateInfo = {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			nullptr,
			0,
			m_image,
			VK_IMAGE_VIEW_TYPE_2D,
			typeToFormat(),
			componentMapping,
			subresourceRange
		};

		if (device.vkCreateImageView(device, &imageviewcreateInfo, nullptr, &m_view) != VK_SUCCESS)
		{
			std::cout << "Could not create texture image view" << std::endl;
		}
	}
}

CVulkanTexture::~CVulkanTexture()
{
	auto& device = CVulkanDevice::get();

	if (m_view)
	{
		device.vkDestroyImageView(device, m_view, nullptr);
	}

	if (m_image)
	{
		device.vkDestroyImage(device, m_image, nullptr);
	}

	m_memoryChunk->freeBlock(m_offset);
}

VkFormat CVulkanTexture::typeToFormat()
{
	switch(m_format)
	{
		case EFormat::eRGB8:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case EFormat::eSRGB8:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case EFormat::eRGB16f:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case EFormat::eDepth32f:
			return VK_FORMAT_D32_SFLOAT;
	}

	return VK_FORMAT_R8G8B8A8_UNORM;
}

VkImageUsageFlags CVulkanTexture::usageFlags()
{
	VkImageUsageFlags usageFlags = 0;

	if (m_usage & EUsage::eSampled)
	{
		usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;

		if (!(m_usage & EUsage::eAttachement))
		{
			usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
	}
	if (m_usage & EUsage::eAttachement)
	{
		if (isFormatDepth())
		{
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else
		{
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
	}

	return usageFlags;
}

size_t CVulkanTexture::getFormatPixelSize()
{
	switch(m_format)
	{
		case EFormat::eRGB8:
			return 4;
		case EFormat::eSRGB8:
			return 4;
		case EFormat::eRGB16f:
			return 16;
		case EFormat::eDepth32f:
			return 4;
	}

	return 4;
}

VkDescriptorImageInfo CVulkanTexture::getDescriptorImageInfo()
{
	return {VK_NULL_HANDLE, m_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
}
