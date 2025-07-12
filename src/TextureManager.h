/**
 * @file TextureManager.h
 * @brief Texture management for compute buffer to graphics texture conversion
 *
 * This class handles the creation of Vulkan textures from compute buffer data,
 * enabling the graphics pipeline to sample fractal data computed by the GPU.
 *
 * Phase 4 Focus:
 * - Compute buffer to texture conversion
 * - Image layout transitions
 * - Texture sampling setup for graphics pipeline
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 4 - Fractal Integration
 */

#pragma once

#include <memory>
#include <vulkan/vulkan.h>

// Forward declarations
class MemoryManager;

/**
 * @class TextureManager
 * @brief Manages texture creation and compute buffer to texture transfers
 *
 * This class provides utilities for creating textures from compute buffer data,
 * handling image layout transitions, and setting up texture sampling for
 * the graphics pipeline.
 */
class TextureManager {
public:
  /**
   * @brief Constructor
   *
   * @param device Vulkan logical device
   * @param physicalDevice Vulkan physical device
   * @param memoryManager Shared memory manager for texture allocation
   */
  TextureManager(VkDevice device, VkPhysicalDevice physicalDevice,
                 std::shared_ptr<MemoryManager> memoryManager);

  /**
   * @brief Destructor - cleanup all textures
   */
  ~TextureManager();

  // Non-copyable
  TextureManager(const TextureManager &) = delete;
  TextureManager &operator=(const TextureManager &) = delete;

  /**
   * @brief Create a texture for fractal data
   *
   * Creates a 2D texture suitable for storing computed fractal data
   * and sampling in the graphics pipeline.
   *
   * @param width Width of the texture
   * @param height Height of the texture
   * @param format Texture format (e.g., VK_FORMAT_R8G8B8A8_UNORM)
   * @return true if successful, false otherwise
   */
  bool createFractalTexture(uint32_t width, uint32_t height, VkFormat format);

  /**
   * @brief Copy data from compute buffer to texture
   *
   * Performs a buffer-to-image copy operation to transfer computed
   * fractal data from the compute buffer to the texture.
   *
   * @param commandBuffer Command buffer to record copy commands
   * @param sourceBuffer Source buffer containing fractal data
   * @param bufferSize Size of the source buffer in bytes
   */
  void copyBufferToTexture(VkCommandBuffer commandBuffer, VkBuffer sourceBuffer,
                           VkDeviceSize bufferSize);

  /**
   * @brief Get the texture image view for binding
   *
   * @return VkImageView for use in descriptor sets
   */
  VkImageView getTextureImageView() const { return m_textureImageView; }

  /**
   * @brief Get the texture sampler for binding
   *
   * @return VkSampler for use in descriptor sets
   */
  VkSampler getTextureSampler() const { return m_textureSampler; }

  /**
   * @brief Get the texture image handle
   *
   * @return VkImage for use in layout transitions
   */
  VkImage getTextureImage() const { return m_textureImage; }

  /**
   * @brief Get the texture format
   *
   * @return VkFormat of the texture
   */
  VkFormat getTextureFormat() const { return m_textureFormat; }

  /**
   * @brief Check if the fractal texture is ready for use
   *
   * @return true if texture is created and ready, false otherwise
   */
  bool isTextureReady() const { return m_textureReady; }

private:
  /**
   * @brief Create the texture sampler
   *
   * @return true if successful, false otherwise
   */
  bool createTextureSampler();

  /**
   * @brief Cleanup texture resources
   */
  void cleanupTexture();

  // Vulkan objects
  VkDevice m_device;
  VkPhysicalDevice m_physicalDevice;
  std::shared_ptr<MemoryManager> m_memoryManager;

  // Texture resources
  VkImage m_textureImage;
  VkDeviceMemory m_textureMemory;
  VkImageView m_textureImageView;
  VkSampler m_textureSampler;

  // Texture properties
  uint32_t m_textureWidth;
  uint32_t m_textureHeight;
  VkFormat m_textureFormat;

  // State
  bool m_textureReady;
};
