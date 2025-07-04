/**
 * @file TextureManager.cpp
 * @brief Implementation of texture management for fractal visualization
 * 
 * This file implements texture creation, buffer-to-texture copying,
 * and image layout transitions for displaying computed fractal data.
 * 
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 4 - Fractal Integration
 */

#include "TextureManager.h"
#include "MemoryManager.h"

#include <iostream>

/**
 * @brief Constructor
 */
TextureManager::TextureManager(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    std::shared_ptr<MemoryManager> memoryManager
)
    : m_device(device)
    , m_physicalDevice(physicalDevice)
    , m_memoryManager(memoryManager)
    , m_textureImage(VK_NULL_HANDLE)
    , m_textureMemory(VK_NULL_HANDLE)
    , m_textureImageView(VK_NULL_HANDLE)
    , m_textureSampler(VK_NULL_HANDLE)
    , m_textureWidth(0)
    , m_textureHeight(0)
    , m_textureFormat(VK_FORMAT_UNDEFINED)
    , m_textureReady(false)
{
    std::cout << "TextureManager: Initializing texture manager..." << std::endl;
}

/**
 * @brief Destructor
 */
TextureManager::~TextureManager() {
    std::cout << "TextureManager: Cleaning up texture resources..." << std::endl;
    cleanupTexture();
}

/**
 * @brief Create a texture for fractal data
 */
bool TextureManager::createFractalTexture(uint32_t width, uint32_t height, VkFormat format) {
    std::cout << "TextureManager: Creating fractal texture (" << width << "x" << height << ")..." << std::endl;
    
    // Store texture properties
    m_textureWidth = width;
    m_textureHeight = height;
    m_textureFormat = format;
    
    // Create the texture image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = vkCreateImage(m_device, &imageInfo, nullptr, &m_textureImage);
    if (result != VK_SUCCESS) {
        std::cerr << "TextureManager: Failed to create texture image! Error: " << result << std::endl;
        return false;
    }
    
    // Allocate memory for the texture
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, m_textureImage, &memRequirements);
    
    // Find suitable memory type
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
    
    uint32_t memoryTypeIndex = UINT32_MAX;
    VkMemoryPropertyFlags requiredProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties) {
            memoryTypeIndex = i;
            break;
        }
    }
    
    if (memoryTypeIndex == UINT32_MAX) {
        std::cerr << "TextureManager: Failed to find suitable memory type for texture!" << std::endl;
        vkDestroyImage(m_device, m_textureImage, nullptr);
        m_textureImage = VK_NULL_HANDLE;
        return false;
    }
    
    // Allocate texture memory
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    result = vkAllocateMemory(m_device, &allocInfo, nullptr, &m_textureMemory);
    if (result != VK_SUCCESS) {
        std::cerr << "TextureManager: Failed to allocate texture memory! Error: " << result << std::endl;
        vkDestroyImage(m_device, m_textureImage, nullptr);
        m_textureImage = VK_NULL_HANDLE;
        return false;
    }
    
    // Bind memory to image
    result = vkBindImageMemory(m_device, m_textureImage, m_textureMemory, 0);
    if (result != VK_SUCCESS) {
        std::cerr << "TextureManager: Failed to bind texture memory! Error: " << result << std::endl;
        vkFreeMemory(m_device, m_textureMemory, nullptr);
        vkDestroyImage(m_device, m_textureImage, nullptr);
        m_textureImage = VK_NULL_HANDLE;
        m_textureMemory = VK_NULL_HANDLE;
        return false;
    }
    
    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    result = vkCreateImageView(m_device, &viewInfo, nullptr, &m_textureImageView);
    if (result != VK_SUCCESS) {
        std::cerr << "TextureManager: Failed to create texture image view! Error: " << result << std::endl;
        vkFreeMemory(m_device, m_textureMemory, nullptr);
        vkDestroyImage(m_device, m_textureImage, nullptr);
        m_textureImage = VK_NULL_HANDLE;
        m_textureMemory = VK_NULL_HANDLE;
        return false;
    }
    
    // Create texture sampler
    if (!createTextureSampler()) {
        vkDestroyImageView(m_device, m_textureImageView, nullptr);
        vkFreeMemory(m_device, m_textureMemory, nullptr);
        vkDestroyImage(m_device, m_textureImage, nullptr);
        m_textureImage = VK_NULL_HANDLE;
        m_textureMemory = VK_NULL_HANDLE;
        m_textureImageView = VK_NULL_HANDLE;
        return false;
    }
    
    m_textureReady = true;
    std::cout << "TextureManager: Fractal texture created successfully." << std::endl;
    return true;
}

/**
 * @brief Copy data from compute buffer to texture
 */
void TextureManager::copyBufferToTexture(
    VkCommandBuffer commandBuffer,
    VkBuffer sourceBuffer,
    VkDeviceSize bufferSize
) {
    // Copy buffer to image
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;  // Tightly packed
    region.bufferImageHeight = 0;  // Tightly packed
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {m_textureWidth, m_textureHeight, 1};
    
    vkCmdCopyBufferToImage(
        commandBuffer,
        sourceBuffer,
        m_textureImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
    
    // Transition texture to shader read-only layout
    transitionTextureLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

/**
 * @brief Transition texture layout for shader access
 */
void TextureManager::transitionTextureLayout(
    VkCommandBuffer commandBuffer,
    VkImageLayout oldLayout,
    VkImageLayout newLayout
) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_textureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        std::cerr << "TextureManager: Unsupported layout transition!" << std::endl;
        return;
    }
    
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

/**
 * @brief Create the texture sampler
 */
bool TextureManager::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    
    VkResult result = vkCreateSampler(m_device, &samplerInfo, nullptr, &m_textureSampler);
    if (result != VK_SUCCESS) {
        std::cerr << "TextureManager: Failed to create texture sampler! Error: " << result << std::endl;
        return false;
    }
    
    return true;
}

/**
 * @brief Cleanup texture resources
 */
void TextureManager::cleanupTexture() {
    if (m_textureSampler != VK_NULL_HANDLE) {
        vkDestroySampler(m_device, m_textureSampler, nullptr);
        m_textureSampler = VK_NULL_HANDLE;
    }
    
    if (m_textureImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device, m_textureImageView, nullptr);
        m_textureImageView = VK_NULL_HANDLE;
    }
    
    if (m_textureMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, m_textureMemory, nullptr);
        m_textureMemory = VK_NULL_HANDLE;
    }
    
    if (m_textureImage != VK_NULL_HANDLE) {
        vkDestroyImage(m_device, m_textureImage, nullptr);
        m_textureImage = VK_NULL_HANDLE;
    }
    
    m_textureReady = false;
}
