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
TextureManager::TextureManager(VkDevice device, VkPhysicalDevice physicalDevice,
                               std::shared_ptr<MemoryManager> memoryManager)
    : m_device(device), m_physicalDevice(physicalDevice),
      m_memoryManager(memoryManager), m_textureImage(VK_NULL_HANDLE),
      m_textureMemory(VK_NULL_HANDLE), m_textureImageView(VK_NULL_HANDLE),
      m_textureSampler(VK_NULL_HANDLE), m_textureWidth(0), m_textureHeight(0),
      m_textureFormat(VK_FORMAT_UNDEFINED), m_textureReady(false) {
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
bool TextureManager::createFractalTexture(uint32_t width, uint32_t height,
                                          VkFormat format) {
  std::cout << "TextureManager: Creating fractal texture (" << width << "x"
            << height << ")..." << std::endl;

  // Store texture properties
  m_textureWidth = width;
  m_textureHeight = height;
  m_textureFormat = format;

  // Create the texture image using MemoryManager utilities - Phase 3
  // integration
  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
  VkImageUsageFlags usage =
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  bool success =
      m_memoryManager->createImage(width, height, format, tiling, usage,
                                   properties, m_textureImage, m_textureMemory);

  if (!success) {
    std::cerr
        << "TextureManager: Failed to create texture image using MemoryManager!"
        << std::endl;
    return false;
  }

  // Create image view using MemoryManager utilities
  m_textureImageView = m_memoryManager->createImageView(
      m_textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT);

  if (m_textureImageView == VK_NULL_HANDLE) {
    std::cerr << "TextureManager: Failed to create texture image view!"
              << std::endl;
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
  std::cout << "TextureManager: Fractal texture created successfully."
            << std::endl;
  return true;
}

/**
 * @brief Copy data from compute buffer to texture
 */
void TextureManager::copyBufferToTexture(VkCommandBuffer commandBuffer,
                                         VkBuffer sourceBuffer,
                                         VkDeviceSize bufferSize) {
  // Copy buffer to image
  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;   // Tightly packed
  region.bufferImageHeight = 0; // Tightly packed
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {m_textureWidth, m_textureHeight, 1};

  vkCmdCopyBufferToImage(commandBuffer, sourceBuffer, m_textureImage,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  // Transition texture to shader read-only layout using MemoryManager utility
  m_memoryManager->transitionImageLayout(
      m_textureImage, m_textureFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);
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

  VkResult result =
      vkCreateSampler(m_device, &samplerInfo, nullptr, &m_textureSampler);
  if (result != VK_SUCCESS) {
    std::cerr << "TextureManager: Failed to create texture sampler! Error: "
              << result << std::endl;
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
