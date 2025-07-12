/**
 * @file MemoryManager.cpp
 * @brief Implementation of Vulkan memory allocation and buffer management
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 2
 */

#include "MemoryManager.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

MemoryManager::MemoryManager(VkDevice device, VkPhysicalDevice physicalDevice)
    : m_device(device), m_physicalDevice(physicalDevice),
      m_totalAllocatedMemory(0) {

  // Get physical device memory properties
  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);

  std::cout << "[MemoryManager] Initialized with "
            << m_memoryProperties.memoryTypeCount << " memory types and "
            << m_memoryProperties.memoryHeapCount << " memory heaps"
            << std::endl;

  // Log available memory heaps for debugging
  for (uint32_t i = 0; i < m_memoryProperties.memoryHeapCount; i++) {
    const auto &heap = m_memoryProperties.memoryHeaps[i];
    std::cout << "[MemoryManager] Heap " << i << ": "
              << (heap.size / (1024 * 1024)) << " MB";
    if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
      std::cout << " (Device Local)";
    }
    std::cout << std::endl;
  }
}

MemoryManager::~MemoryManager() {
  std::cout << "[MemoryManager] Cleaning up " << m_buffers.size()
            << " buffers..." << std::endl;
  clearBuffers();
  std::cout << "[MemoryManager] Cleanup complete. Total memory freed: "
            << (m_totalAllocatedMemory / (1024 * 1024)) << " MB" << std::endl;
}

std::shared_ptr<BufferInfo> MemoryManager::createBuffer(const std::string &name,
                                                        VkDeviceSize size,
                                                        BufferUsage usage,
                                                        MemoryLocation location,
                                                        bool persistentMap) {
  // Convert enums to Vulkan flags
  VkBufferUsageFlags usageFlags = bufferUsageToVulkanFlags(usage);
  VkMemoryPropertyFlags memoryProperties =
      memoryLocationToVulkanFlags(location);

  return createBufferExplicit(name, size, usageFlags, memoryProperties,
                              persistentMap);
}

std::shared_ptr<BufferInfo> MemoryManager::createBufferExplicit(
    const std::string &name, VkDeviceSize size, VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryProperties, bool persistentMap) {
  std::cout << "[MemoryManager] Creating buffer '" << name
            << "' (size: " << (size / 1024) << " KB)" << std::endl;

  // Check if buffer already exists
  if (getBuffer(name)) {
    throw std::runtime_error("Buffer with name '" + name + "' already exists");
  }

  // Create buffer info structure
  auto bufferInfo = std::make_shared<BufferInfo>();
  bufferInfo->size = size;
  bufferInfo->offset = 0;
  bufferInfo->mappedData = nullptr;
  bufferInfo->persistentlyMapped = persistentMap;

  try {
    // Create Vulkan buffer
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(m_device, &bufferCreateInfo, nullptr,
                                     &bufferInfo->buffer);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to create buffer! Vulkan error: " +
                               std::to_string(result));
    }

    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, bufferInfo->buffer,
                                  &memRequirements);

    // Find suitable memory type
    uint32_t memoryTypeIndex =
        findMemoryType(memRequirements.memoryTypeBits, memoryProperties);

    // Allocate memory
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    result =
        vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferInfo->memory);
    if (result != VK_SUCCESS) {
      vkDestroyBuffer(m_device, bufferInfo->buffer, nullptr);
      throw std::runtime_error(
          "Failed to allocate buffer memory! Vulkan error: " +
          std::to_string(result));
    }

    // Bind buffer to memory
    result =
        vkBindBufferMemory(m_device, bufferInfo->buffer, bufferInfo->memory, 0);
    if (result != VK_SUCCESS) {
      vkFreeMemory(m_device, bufferInfo->memory, nullptr);
      vkDestroyBuffer(m_device, bufferInfo->buffer, nullptr);
      throw std::runtime_error("Failed to bind buffer memory! Vulkan error: " +
                               std::to_string(result));
    }

    // Update tracking
    m_totalAllocatedMemory += memRequirements.size;

    // Map memory if requested and possible
    if (persistentMap &&
        (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
      bufferInfo->mappedData = mapBuffer(bufferInfo);
    }

    // Cache the buffer
    m_buffers[name] = bufferInfo;

    std::cout << "[MemoryManager] Successfully created buffer '" << name
              << "' (allocated: " << (memRequirements.size / 1024) << " KB, "
              << "total: " << (m_totalAllocatedMemory / (1024 * 1024)) << " MB)"
              << std::endl;

    return bufferInfo;

  } catch (const std::exception &e) {
    std::cerr << "[MemoryManager] Failed to create buffer '" << name
              << "': " << e.what() << std::endl;
    throw;
  }
}

void MemoryManager::uploadBufferData(std::shared_ptr<BufferInfo> buffer,
                                     const void *data, VkDeviceSize size,
                                     VkDeviceSize offset,
                                     VkCommandPool commandPool, VkQueue queue) {
  if (!buffer) {
    throw std::runtime_error("Invalid buffer for upload operation");
  }

  if (offset + size > buffer->size) {
    throw std::runtime_error("Upload data exceeds buffer size");
  }

  // Check if buffer memory is host-visible (can be mapped)
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(m_device, buffer->buffer, &memRequirements);

  VkMemoryPropertyFlags memFlags;
  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);

  // Find the memory type index for this buffer
  for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++) {
    if ((memRequirements.memoryTypeBits & (1 << i))) {
      // Get the memory properties for this type
      VkDeviceMemory memory;
      VkMemoryAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocInfo.memoryTypeIndex = i;

      memFlags = m_memoryProperties.memoryTypes[i].propertyFlags;
      break;
    }
  }

  if (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
    // Direct copy for host-visible memory
    void *mappedMemory;
    VkResult result =
        vkMapMemory(m_device, buffer->memory, offset, size, 0, &mappedMemory);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to map buffer memory for upload");
    }

    std::memcpy(mappedMemory, data, size);

    // Flush if not coherent
    if (!(memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
      VkMappedMemoryRange range{};
      range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      range.memory = buffer->memory;
      range.offset = offset;
      range.size = size;
      vkFlushMappedMemoryRanges(m_device, 1, &range);
    }

    vkUnmapMemory(m_device, buffer->memory);

  } else {
    // Use staging buffer for device-local memory
    if (commandPool == VK_NULL_HANDLE || queue == VK_NULL_HANDLE) {
      throw std::runtime_error(
          "Command pool and queue required for staging buffer upload");
    }

    // Create temporary staging buffer
    auto stagingBuffer = createStagingBuffer(size);

    // Copy data to staging buffer
    void *mappedMemory;
    VkResult result =
        vkMapMemory(m_device, stagingBuffer->memory, 0, size, 0, &mappedMemory);
    if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to map staging buffer memory");
    }

    std::memcpy(mappedMemory, data, size);
    vkUnmapMemory(m_device, stagingBuffer->memory);

    // Copy from staging buffer to target buffer
    copyBufferToBuffer(stagingBuffer->buffer, buffer->buffer, size, 0, offset,
                       commandPool, queue);
  }
}

void MemoryManager::downloadBufferData(std::shared_ptr<BufferInfo> buffer,
                                       void *data, VkDeviceSize size,
                                       VkDeviceSize offset) {
  if (!buffer) {
    throw std::runtime_error("Invalid buffer for download operation");
  }

  if (offset + size > buffer->size) {
    throw std::runtime_error("Download size exceeds buffer size");
  }

  // Map memory and copy data
  void *mappedMemory;
  VkResult result =
      vkMapMemory(m_device, buffer->memory, offset, size, 0, &mappedMemory);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to map buffer memory for download (buffer "
                             "may not be host-visible)");
  }

  std::memcpy(data, mappedMemory, size);
  vkUnmapMemory(m_device, buffer->memory);
}

void *MemoryManager::mapBuffer(std::shared_ptr<BufferInfo> buffer) {
  if (!buffer) {
    throw std::runtime_error("Invalid buffer for mapping");
  }

  if (buffer->mappedData) {
    return buffer->mappedData; // Already mapped
  }

  VkResult result = vkMapMemory(m_device, buffer->memory, 0, buffer->size, 0,
                                &buffer->mappedData);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to map buffer memory");
  }

  return buffer->mappedData;
}

void MemoryManager::unmapBuffer(std::shared_ptr<BufferInfo> buffer) {
  if (!buffer || !buffer->mappedData) {
    return; // Not mapped or invalid buffer
  }

  if (!buffer->persistentlyMapped) {
    vkUnmapMemory(m_device, buffer->memory);
    buffer->mappedData = nullptr;
  }
}

std::shared_ptr<BufferInfo>
MemoryManager::getBuffer(const std::string &name) const {
  auto it = m_buffers.find(name);
  if (it != m_buffers.end()) {
    return it->second;
  }
  return nullptr;
}

bool MemoryManager::removeBuffer(const std::string &name) {
  auto it = m_buffers.find(name);
  if (it != m_buffers.end()) {
    std::cout << "[MemoryManager] Removing buffer: " << name << std::endl;

    auto buffer = it->second;

    // Unmap if mapped
    if (buffer->mappedData) {
      vkUnmapMemory(m_device, buffer->memory);
    }

    // Destroy Vulkan objects
    vkDestroyBuffer(m_device, buffer->buffer, nullptr);
    vkFreeMemory(m_device, buffer->memory, nullptr);

    // Update tracking
    m_totalAllocatedMemory -= buffer->size;

    // Remove from cache
    m_buffers.erase(it);
    return true;
  }
  return false;
}

void MemoryManager::clearBuffers() {
  for (const auto &[name, buffer] : m_buffers) {
    std::cout << "[MemoryManager] Destroying buffer: " << name << std::endl;

    // Unmap if mapped
    if (buffer->mappedData) {
      vkUnmapMemory(m_device, buffer->memory);
    }

    // Destroy Vulkan objects
    vkDestroyBuffer(m_device, buffer->buffer, nullptr);
    vkFreeMemory(m_device, buffer->memory, nullptr);
  }

  m_buffers.clear();
  m_totalAllocatedMemory = 0;
}

std::vector<std::string> MemoryManager::getBufferNames() const {
  std::vector<std::string> names;
  names.reserve(m_buffers.size());

  for (const auto &[name, buffer] : m_buffers) {
    names.push_back(name);
  }

  return names;
}

VkDeviceSize MemoryManager::getTotalAllocatedMemory() const {
  return m_totalAllocatedMemory;
}

uint32_t MemoryManager::findMemoryType(uint32_t typeFilter,
                                       VkMemoryPropertyFlags properties) {
  for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) &&
        (m_memoryProperties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i;
    }
  }

  throw std::runtime_error("Failed to find suitable memory type!");
}

VkBufferUsageFlags MemoryManager::bufferUsageToVulkanFlags(BufferUsage usage) {
  switch (usage) {
  case BufferUsage::VERTEX_BUFFER:
    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  case BufferUsage::INDEX_BUFFER:
    return VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  case BufferUsage::UNIFORM_BUFFER:
    return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
           VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  case BufferUsage::STORAGE_BUFFER:
    return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  case BufferUsage::STAGING_BUFFER:
    return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  case BufferUsage::FRACTAL_OUTPUT_BUFFER:
    return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
           VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  case BufferUsage::FRACTAL_PARAMS_BUFFER:
    return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
           VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  default:
    throw std::runtime_error("Unsupported buffer usage type");
  }
}

VkMemoryPropertyFlags
MemoryManager::memoryLocationToVulkanFlags(MemoryLocation location) {
  switch (location) {
  case MemoryLocation::GPU_ONLY:
    return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  case MemoryLocation::CPU_TO_GPU:
    return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  case MemoryLocation::GPU_TO_CPU:
    return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
           VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
  case MemoryLocation::CPU_GPU_SHARED:
    return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  default:
    throw std::runtime_error("Unsupported memory location type");
  }
}

std::shared_ptr<BufferInfo>
MemoryManager::createStagingBuffer(VkDeviceSize size) {
  static int stagingCount = 0;
  std::string name = "staging_" + std::to_string(stagingCount++);

  return createBufferExplicit(name, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              false);
}

void MemoryManager::copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                                       VkDeviceSize size,
                                       VkDeviceSize srcOffset,
                                       VkDeviceSize dstOffset,
                                       VkCommandPool commandPool,
                                       VkQueue queue) {
  // Allocate command buffer
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

  // Record copy command
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion{};
  copyRegion.srcOffset = srcOffset;
  copyRegion.dstOffset = dstOffset;
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(commandBuffer);

  // Submit and wait
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(queue);

  // Free command buffer
  vkFreeCommandBuffers(m_device, commandPool, 1, &commandBuffer);
}

bool MemoryManager::createImage(uint32_t width, uint32_t height,
                                VkFormat format, VkImageTiling tiling,
                                VkImageUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkImage &image, VkDeviceMemory &imageMemory) {
  // Create image
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    std::cerr << "[MemoryManager] Failed to create image!" << std::endl;
    return false;
  }

  // Get memory requirements
  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(m_device, image, &memRequirements);

  // Find appropriate memory type
  uint32_t memoryTypeIndex;
  try {
    memoryTypeIndex =
        findMemoryType(memRequirements.memoryTypeBits, properties);
  } catch (const std::runtime_error &e) {
    std::cerr
        << "[MemoryManager] Failed to find suitable memory type for image: "
        << e.what() << std::endl;
    vkDestroyImage(m_device, image, nullptr);
    return false;
  }

  // Allocate memory
  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = memoryTypeIndex;

  if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) !=
      VK_SUCCESS) {
    std::cerr << "[MemoryManager] Failed to allocate image memory!"
              << std::endl;
    vkDestroyImage(m_device, image, nullptr);
    return false;
  }

  // Bind memory to image
  vkBindImageMemory(m_device, image, imageMemory, 0);

  std::cout << "[MemoryManager] Created image (" << width << "x" << height
            << ") with " << (memRequirements.size / 1024) << " KB memory"
            << std::endl;

  return true;
}

VkImageView MemoryManager::createImageView(VkImage image, VkFormat format,
                                           VkImageAspectFlags aspectFlags) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  if (vkCreateImageView(m_device, &viewInfo, nullptr, &imageView) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create image view!");
  }

  return imageView;
}

void MemoryManager::transitionImageLayout(VkImage image, VkFormat format,
                                          VkImageLayout oldLayout,
                                          VkImageLayout newLayout,
                                          VkCommandBuffer commandBuffer) {
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw std::invalid_argument("Unsupported layout transition!");
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);
}

void MemoryManager::copyBufferToImage(VkBuffer buffer, VkImage image,
                                      uint32_t width, uint32_t height,
                                      VkCommandBuffer commandBuffer) {
  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}
