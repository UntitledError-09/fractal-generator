/**
 * @file MemoryManager.h
 * @brief Vulkan memory allocation and buffer management utilities
 *
 * This class provides high-level utilities for Vulkan memory management,
 * including buffer creation, memory allocation, and data transfer operations.
 * It simplifies the complex Vulkan memory management APIs.
 *
 * Phase 2 Focus:
 * - Buffer creation and memory binding
 * - Memory type selection and allocation
 * - Data transfer utilities (staging buffers)
 * - Resource cleanup and management
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 2
 */

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

/**
 * @enum BufferUsage
 * @brief Common buffer usage patterns for fractal generation
 */
enum class BufferUsage {
  VERTEX_BUFFER,         ///< Vertex data for rendering
  INDEX_BUFFER,          ///< Index data for indexed rendering
  UNIFORM_BUFFER,        ///< Uniform data (parameters, matrices)
  STORAGE_BUFFER,        ///< General storage for compute shaders
  STAGING_BUFFER,        ///< Temporary buffer for data transfer
  FRACTAL_OUTPUT_BUFFER, ///< Output buffer for fractal computation
  FRACTAL_PARAMS_BUFFER  ///< Parameters for fractal computation
};

/**
 * @enum MemoryLocation
 * @brief Memory location preferences for different use cases
 */
enum class MemoryLocation {
  GPU_ONLY,      ///< Device local memory (fastest for GPU)
  CPU_TO_GPU,    ///< Host visible, for CPU->GPU transfers
  GPU_TO_CPU,    ///< Host cached, for GPU->CPU readback
  CPU_GPU_SHARED ///< Host coherent, for frequent updates
};

/**
 * @struct BufferInfo
 * @brief Information about an allocated buffer
 */
struct BufferInfo {
  VkBuffer buffer;         ///< Vulkan buffer handle
  VkDeviceMemory memory;   ///< Allocated device memory
  VkDeviceSize size;       ///< Size of the buffer in bytes
  VkDeviceSize offset;     ///< Offset in the device memory
  void *mappedData;        ///< Mapped host pointer (if applicable)
  BufferUsage usage;       ///< Intended usage of the buffer
  MemoryLocation location; ///< Memory location type
  bool persistentlyMapped; ///< Whether the buffer stays mapped
};

/**
 * @class MemoryManager
 * @brief High-level Vulkan memory management utilities
 *
 * This class wraps Vulkan's verbose memory management APIs with a more
 * user-friendly interface. It handles memory type selection, buffer
 * creation, and data transfer operations.
 *
 * Key Features:
 * - Automatic memory type selection based on usage
 * - Buffer creation with appropriate usage flags
 * - Staging buffer management for efficient transfers
 * - Memory mapping for host-visible buffers
 * - Resource tracking and automatic cleanup
 *
 * Design Philosophy:
 * - Hide Vulkan memory complexity behind simple APIs
 * - Optimize for fractal generation workloads
 * - Provide both convenience and control when needed
 * - Prepared for future advanced memory management
 */
class MemoryManager {
public:
  /**
   * @brief Constructor - Initialize memory manager
   *
   * @param device Vulkan logical device
   * @param physicalDevice Vulkan physical device for memory properties
   */
  MemoryManager(VkDevice device, VkPhysicalDevice physicalDevice);

  /**
   * @brief Destructor - Clean up all allocated resources
   */
  ~MemoryManager();

  // Disable copy and move for simplicity
  MemoryManager(const MemoryManager &) = delete;
  MemoryManager &operator=(const MemoryManager &) = delete;
  MemoryManager(MemoryManager &&) = delete;
  MemoryManager &operator=(MemoryManager &&) = delete;

  /**
   * @brief Create a buffer with automatic memory allocation
   *
   * @param name Unique name for the buffer (for tracking and debugging)
   * @param size Size of the buffer in bytes
   * @param usage Intended usage of the buffer
   * @param location Memory location preference
   * @param persistentMap Whether to keep the buffer mapped (for CPU-visible
   * memory)
   * @return Pointer to BufferInfo containing buffer details
   *
   * @throws std::runtime_error If buffer creation or memory allocation fails
   */
  std::shared_ptr<BufferInfo> createBuffer(const std::string &name,
                                           VkDeviceSize size, BufferUsage usage,
                                           MemoryLocation location,
                                           bool persistentMap = false);

  /**
   * @brief Create a buffer with explicit Vulkan usage flags
   *
   * @param name Unique name for the buffer
   * @param size Size of the buffer in bytes
   * @param usageFlags Vulkan buffer usage flags
   * @param memoryProperties Required memory property flags
   * @param persistentMap Whether to keep the buffer mapped
   * @return Pointer to BufferInfo containing buffer details
   *
   * @throws std::runtime_error If buffer creation or memory allocation fails
   */
  std::shared_ptr<BufferInfo> createBufferExplicit(
      const std::string &name, VkDeviceSize size, VkBufferUsageFlags usageFlags,
      VkMemoryPropertyFlags memoryProperties, bool persistentMap = false);

  /**
   * @brief Upload data to a buffer using staging if necessary
   *
   * Automatically handles staging buffer creation for GPU-only memory.
   * For host-visible memory, copies directly.
   *
   * @param buffer Target buffer to upload to
   * @param data Source data pointer
   * @param size Size of data to upload
   * @param offset Offset in the target buffer
   * @param commandPool Command pool for staging operations
   * @param queue Queue for submitting transfer commands
   *
   * @throws std::runtime_error If upload fails
   */
  void uploadBufferData(std::shared_ptr<BufferInfo> buffer, const void *data,
                        VkDeviceSize size, VkDeviceSize offset = 0,
                        VkCommandPool commandPool = VK_NULL_HANDLE,
                        VkQueue queue = VK_NULL_HANDLE);

  /**
   * @brief Download data from a buffer to host memory
   *
   * @param buffer Source buffer to download from
   * @param data Destination host memory
   * @param size Size of data to download
   * @param offset Offset in the source buffer
   *
   * @throws std::runtime_error If buffer is not host-visible or download fails
   */
  void downloadBufferData(std::shared_ptr<BufferInfo> buffer, void *data,
                          VkDeviceSize size, VkDeviceSize offset = 0);

  /**
   * @brief Map buffer memory for host access
   *
   * @param buffer Buffer to map
   * @return Pointer to mapped memory
   *
   * @throws std::runtime_error If buffer is not host-visible or mapping fails
   */
  void *mapBuffer(std::shared_ptr<BufferInfo> buffer);

  /**
   * @brief Unmap previously mapped buffer memory
   *
   * @param buffer Buffer to unmap
   */
  void unmapBuffer(std::shared_ptr<BufferInfo> buffer);

  /**
   * @brief Get buffer by name
   *
   * @param name Name of the buffer to retrieve
   * @return Pointer to BufferInfo if found, nullptr otherwise
   */
  std::shared_ptr<BufferInfo> getBuffer(const std::string &name) const;

  /**
   * @brief Remove buffer and free its memory
   *
   * @param name Name of the buffer to remove
   * @return true if buffer was found and removed, false otherwise
   */
  bool removeBuffer(const std::string &name);

  /**
   * @brief Clear all buffers and free memory
   */
  void clearBuffers();

  /**
   * @brief Get list of all buffer names
   *
   * @return Vector of buffer names
   */
  std::vector<std::string> getBufferNames() const;

  /**
   * @brief Get total allocated memory size
   *
   * @return Total size of allocated device memory in bytes
   */
  VkDeviceSize getTotalAllocatedMemory() const;

  /**
   * @brief Create a Vulkan image with appropriate memory allocation
   *
   * @param width Image width in pixels
   * @param height Image height in pixels
   * @param format Image format (e.g., VK_FORMAT_R8G8B8A8_UNORM)
   * @param tiling Image tiling mode
   * @param usage Image usage flags
   * @param properties Memory property flags
   * @param image Output image handle
   * @param imageMemory Output allocated memory handle
   * @return True if creation was successful
   */
  bool createImage(uint32_t width, uint32_t height, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage &image,
                   VkDeviceMemory &imageMemory);

  /**
   * @brief Create an image view for an existing image
   *
   * @param image Source image handle
   * @param format Image format
   * @param aspectFlags Image aspect flags
   * @return Created image view handle
   */
  VkImageView createImageView(VkImage image, VkFormat format,
                              VkImageAspectFlags aspectFlags);

  /**
   * @brief Transition image layout using a command buffer
   *
   * @param image Image to transition
   * @param format Image format
   * @param oldLayout Current layout
   * @param newLayout Target layout
   * @param commandBuffer Command buffer to record commands into
   */
  void transitionImageLayout(VkImage image, VkFormat format,
                             VkImageLayout oldLayout, VkImageLayout newLayout,
                             VkCommandBuffer commandBuffer);

  /**
   * @brief Copy data from buffer to image
   *
   * @param buffer Source buffer
   * @param image Target image
   * @param width Image width
   * @param height Image height
   * @param commandBuffer Command buffer to record commands into
   */
  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                         uint32_t height, VkCommandBuffer commandBuffer);

  // TODO(Phase 4): Add memory pool management for efficiency
  // TODO(Phase 5): Add memory budget tracking and optimization

private:
  /**
   * @brief Find suitable memory type for buffer requirements
   *
   * @param typeFilter Bitmask of suitable memory types
   * @param properties Required memory properties
   * @return Index of suitable memory type
   *
   * @throws std::runtime_error If no suitable memory type found
   */
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);

  /**
   * @brief Convert BufferUsage to Vulkan buffer usage flags
   *
   * @param usage BufferUsage enum value
   * @return Corresponding VkBufferUsageFlags
   */
  VkBufferUsageFlags bufferUsageToVulkanFlags(BufferUsage usage);

  /**
   * @brief Convert MemoryLocation to Vulkan memory property flags
   *
   * @param location MemoryLocation enum value
   * @return Corresponding VkMemoryPropertyFlags
   */
  VkMemoryPropertyFlags memoryLocationToVulkanFlags(MemoryLocation location);

  /**
   * @brief Create a temporary staging buffer for transfers
   *
   * @param size Size of the staging buffer
   * @return BufferInfo for the staging buffer
   */
  std::shared_ptr<BufferInfo> createStagingBuffer(VkDeviceSize size);

  /**
   * @brief Copy data between buffers using command buffer
   *
   * @param srcBuffer Source buffer
   * @param dstBuffer Destination buffer
   * @param size Size of data to copy
   * @param srcOffset Offset in source buffer
   * @param dstOffset Offset in destination buffer
   * @param commandPool Command pool for operations
   * @param queue Queue for command submission
   */
  void copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                          VkDeviceSize size, VkDeviceSize srcOffset = 0,
                          VkDeviceSize dstOffset = 0,
                          VkCommandPool commandPool = VK_NULL_HANDLE,
                          VkQueue queue = VK_NULL_HANDLE);

  // Member variables
  VkDevice m_device;                 ///< Vulkan logical device
  VkPhysicalDevice m_physicalDevice; ///< Vulkan physical device
  VkPhysicalDeviceMemoryProperties
      m_memoryProperties; ///< Device memory properties

  std::unordered_map<std::string, std::shared_ptr<BufferInfo>>
      m_buffers;                       ///< Tracked buffers
  VkDeviceSize m_totalAllocatedMemory; ///< Total allocated memory
};

/**
 * Implementation Notes:
 *
 * 1. Memory Management Strategy:
 *    - Automatic memory type selection based on usage patterns
 *    - Staging buffers for efficient GPU memory transfers
 *    - Persistent mapping for frequently updated buffers
 *    - Resource tracking for automatic cleanup
 *
 * 2. Buffer Usage Patterns:
 *    - Fractal output buffers in device-local memory for speed
 *    - Parameter buffers in host-visible memory for frequent updates
 *    - Staging buffers for one-time transfers
 *
 * 3. Performance Considerations:
 *    - Minimize memory allocations through reuse
 *    - Use appropriate memory types for access patterns
 *    - Batch transfers to reduce command buffer overhead
 *
 * 4. Error Handling:
 *    - Clear error messages for common memory issues
 *    - Graceful fallbacks for memory allocation failures
 *    - Validation of memory requirements and availability
 *
 * 5. Future Extensions:
 *    - Memory pools for efficient small allocations
 *    - Budget tracking for mobile/integrated GPUs
 *    - Advanced transfer scheduling and optimization
 */
