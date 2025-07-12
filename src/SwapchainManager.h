/**
 * @file SwapchainManager.h
 * @brief Vulkan swapchain management for window presentation
 *
 * This class handles swapchain creation, recreation, and presentation
 * for displaying rendered images to the window. It manages multiple
 * framebuffers for double/triple buffering.
 *
 * Phase 3 Focus:
 * - Swapchain creation and configuration
 * - Surface format and present mode selection
 * - Framebuffer management
 * - Window resize handling
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 3
 */

#pragma once

#include <vector>
#include <vulkan/vulkan.h>

// Forward declarations
class WindowManager;

/**
 * @struct SwapchainSupportDetails
 * @brief Details about swapchain support for a device
 */
struct SwapchainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;      ///< Surface capabilities
  std::vector<VkSurfaceFormatKHR> formats;    ///< Available surface formats
  std::vector<VkPresentModeKHR> presentModes; ///< Available present modes
};

/**
 * @class SwapchainManager
 * @brief Manages Vulkan swapchain for window presentation
 *
 * This class provides a complete swapchain management system for
 * presenting rendered images to the window. It handles format selection,
 * resize events, and optimal performance configuration.
 *
 * Key Features:
 * - Automatic format and present mode selection
 * - Window resize handling with swapchain recreation
 * - Multiple framebuffer management
 * - Optimal performance configuration for target platform
 * - Integration with existing Vulkan setup
 */
class SwapchainManager {
public:
  /**
   * @brief Constructor - Initialize swapchain manager
   *
   * @param device Vulkan logical device
   * @param physicalDevice Vulkan physical device
   * @param surface Window surface
   * @param windowManager Window manager for dimensions
   */
  SwapchainManager(VkDevice device, VkPhysicalDevice physicalDevice,
                   VkSurfaceKHR surface, const WindowManager &windowManager);

  /**
   * @brief Destructor - Clean up swapchain resources
   */
  ~SwapchainManager();

  // Disable copy and move for simplicity
  SwapchainManager(const SwapchainManager &) = delete;
  SwapchainManager &operator=(const SwapchainManager &) = delete;
  SwapchainManager(SwapchainManager &&) = delete;
  SwapchainManager &operator=(SwapchainManager &&) = delete;

  /**
   * @brief Create swapchain and related resources
   *
   * @return true if swapchain created successfully, false otherwise
   */
  bool createSwapchain();

  /**
   * @brief Recreate swapchain (e.g., after window resize)
   *
   * @return true if swapchain recreated successfully, false otherwise
   */
  bool recreateSwapchain();

  /**
   * @brief Acquire next swapchain image for rendering
   *
   * @param semaphore Semaphore to signal when image is available
   * @param imageIndex Output parameter for acquired image index
   * @return VkResult indicating success or specific error
   */
  VkResult acquireNextImage(VkSemaphore semaphore, uint32_t &imageIndex);

  /**
   * @brief Present rendered image to screen
   *
   * @param presentQueue Queue for presentation
   * @param imageIndex Index of image to present
   * @param waitSemaphore Semaphore to wait for before presenting
   * @return VkResult indicating success or specific error
   */
  VkResult presentImage(VkQueue presentQueue, uint32_t imageIndex,
                        VkSemaphore waitSemaphore);

  /**
   * @brief Get swapchain handle
   * @return VkSwapchainKHR handle
   */
  VkSwapchainKHR getSwapchain() const { return m_swapchain; }

  /**
   * @brief Get swapchain images
   * @return Vector of swapchain image handles
   */
  const std::vector<VkImage> &getImages() const { return m_images; }

  /**
   * @brief Get swapchain image views
   * @return Vector of swapchain image view handles
   */
  const std::vector<VkImageView> &getImageViews() const { return m_imageViews; }

  /**
   * @brief Get swapchain extent (dimensions)
   * @return VkExtent2D containing width and height
   */
  VkExtent2D getExtent() const { return m_extent; }

  /**
   * @brief Get swapchain format
   * @return VkFormat of swapchain images
   */
  VkFormat getFormat() const { return m_format; }

  /**
   * @brief Get number of swapchain images
   * @return Number of images in swapchain
   */
  uint32_t getImageCount() const {
    return static_cast<uint32_t>(m_images.size());
  }

private:
  /**
   * @brief Query swapchain support details for the physical device
   *
   * @return SwapchainSupportDetails structure with capabilities and formats
   */
  SwapchainSupportDetails querySwapchainSupport();

  /**
   * @brief Choose optimal surface format
   *
   * @param availableFormats Available surface formats
   * @return Selected VkSurfaceFormatKHR
   */
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);

  /**
   * @brief Choose optimal present mode
   *
   * @param availablePresentModes Available present modes
   * @return Selected VkPresentModeKHR
   */
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);

  /**
   * @brief Choose swapchain extent (dimensions)
   *
   * @param capabilities Surface capabilities
   * @return Selected VkExtent2D
   */
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  /**
   * @brief Create image views for swapchain images
   */
  void createImageViews();

  /**
   * @brief Clean up swapchain resources
   */
  void cleanupSwapchain();

  // Member variables
  VkDevice m_device;                    ///< Vulkan logical device
  VkPhysicalDevice m_physicalDevice;    ///< Vulkan physical device
  VkSurfaceKHR m_surface;               ///< Window surface
  const WindowManager &m_windowManager; ///< Window manager reference

  VkSwapchainKHR m_swapchain;            ///< Swapchain handle
  std::vector<VkImage> m_images;         ///< Swapchain images
  std::vector<VkImageView> m_imageViews; ///< Swapchain image views
  VkFormat m_format;                     ///< Swapchain image format
  VkExtent2D m_extent;                   ///< Swapchain dimensions
};

/**
 * Implementation Notes:
 *
 * 1. Format Selection:
 *    - Prefer BGRA8 SRGB for optimal performance on most platforms
 *    - Fallback to first available format if preferred not available
 *    - Consider HDR formats for future enhancement
 *
 * 2. Present Mode Selection:
 *    - Prefer MAILBOX for low latency with vsync
 *    - Fallback to FIFO (guaranteed to be available)
 *    - Consider IMMEDIATE for maximum performance when needed
 *
 * 3. Swapchain Recreation:
 *    - Handle window resize events gracefully
 *    - Minimize resource allocation during recreation
 *    - Proper synchronization during transition
 *
 * 4. Performance Considerations:
 *    - Triple buffering when available for smooth rendering
 *    - Optimal extent selection for current window size
 *    - Efficient image acquisition and presentation
 */
