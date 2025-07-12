/**
 * @file VulkanSetup.h
 * @brief Vulkan initialization and device management
 *
 * This class handles all Vulkan initialization including instance creation,
 * physical device selection, logical device creation, and basic resource
 * management. It provides a clean abstraction over Vulkan's verbose setup
 * process.
 *
 * Phase 1 Focus:
 * - Vulkan instance creation with validation layers
 * - Physical device selection with scoring
 * - Logical device creation with appropriate queues
 * - Basic error handling and resource management
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 1
 */

#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

// Forward declaration
class WindowManager;

/**
 * @struct QueueFamilyIndices
 * @brief Holds indices for different types of queue families
 *
 * Different types of GPU operations require different queue families.
 * This structure tracks the indices of queue families that support
 * the operations we need for fractal generation.
 */
struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily; ///< Graphics operations (rendering)
  std::optional<uint32_t>
      computeFamily; ///< Compute operations (fractal calculation)
  std::optional<uint32_t> presentFamily;  ///< Presentation to window surface
  std::optional<uint32_t> transferFamily; ///< Memory transfer operations

  /**
   * @brief Check if all required queue families are available
   * @return true if all required families found, false otherwise
   */
  bool isComplete() const {
    return graphicsFamily.has_value() && computeFamily.has_value() &&
           presentFamily.has_value();
    // transferFamily is optional - can use graphics queue if needed
  }
};

/**
 * @struct PhysicalDeviceInfo
 * @brief Information about a physical device for selection scoring
 *
 * Contains relevant information for scoring physical devices to select
 * the best one for our fractal generation workload.
 */
struct PhysicalDeviceInfo {
  VkPhysicalDevice device; ///< Vulkan physical device handle
  VkPhysicalDeviceProperties
      properties;                    ///< Device properties (name, type, etc.)
  VkPhysicalDeviceFeatures features; ///< Supported features
  QueueFamilyIndices queueFamilies;  ///< Available queue families
  std::vector<VkExtensionProperties> extensions; ///< Supported extensions
  uint32_t score; ///< Calculated suitability score
};

/**
 * @class VulkanSetup
 * @brief Manages Vulkan initialization and device selection
 *
 * This class encapsulates the complex Vulkan initialization process,
 * providing a clean interface for the application to access Vulkan
 * functionality. It follows RAII principles for resource management.
 *
 * Key Responsibilities:
 * - Vulkan instance creation with appropriate extensions and layers
 * - Physical device enumeration and selection
 * - Logical device creation with required queues
 * - Basic resource management and cleanup
 *
 * Design Philosophy:
 * - Hide Vulkan complexity behind a clean interface
 * - Extensive validation and error checking
 * - Educational comments explaining Vulkan concepts
 * - Prepared for future extension (compute pipelines, etc.)
 */
class VulkanSetup {
public:
  /**
   * @brief Constructor - Initialize Vulkan for the given window
   *
   * Performs complete Vulkan initialization including:
   * - Instance creation with validation layers (debug builds)
   * - Physical device selection based on requirements
   * - Logical device creation with necessary queues
   * - Surface creation for window presentation
   *
   * @param windowManager Window manager for surface creation
   *
   * @throws std::runtime_error If Vulkan initialization fails
   * @throws VulkanException For Vulkan-specific errors
   */
  explicit VulkanSetup(const WindowManager &windowManager);

  /**
   * @brief Destructor - Clean up all Vulkan resources
   *
   * Ensures proper cleanup order:
   * 1. Device wait for idle
   * 2. Destroy logical device
   * 3. Destroy surface
   * 4. Destroy debug messenger (if enabled)
   * 5. Destroy instance
   */
  ~VulkanSetup();

  // Disable copy and move for simplicity in Phase 1
  // Vulkan objects are not trivially copyable/movable
  VulkanSetup(const VulkanSetup &) = delete;
  VulkanSetup &operator=(const VulkanSetup &) = delete;
  VulkanSetup(VulkanSetup &&) = delete;
  VulkanSetup &operator=(VulkanSetup &&) = delete;

  // Accessors for Vulkan objects

  /**
   * @brief Get the Vulkan instance
   * @return VkInstance handle
   */
  VkInstance getInstance() const { return m_instance; }

  /**
   * @brief Get the physical device
   * @return VkPhysicalDevice handle
   */
  VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

  /**
   * @brief Get the logical device
   * @return VkDevice handle
   */
  VkDevice getDevice() const { return m_device; }

  /**
   * @brief Get the window surface
   * @return VkSurfaceKHR handle
   */
  VkSurfaceKHR getSurface() const { return m_surface; }

  /**
   * @brief Get queue family indices
   * @return QueueFamilyIndices structure
   */
  const QueueFamilyIndices &getQueueFamilies() const { return m_queueFamilies; }

  /**
   * @brief Get graphics queue family index
   *
   * @return Graphics queue family index
   */
  uint32_t getGraphicsQueueFamily() const {
    return m_queueFamilies.graphicsFamily.value();
  }

  /**
   * @brief Get the graphics queue
   * @return VkQueue handle for graphics operations
   */
  VkQueue getGraphicsQueue() const { return m_graphicsQueue; }

  /**
   * @brief Get the compute queue
   * @return VkQueue handle for compute operations
   */
  VkQueue getComputeQueue() const { return m_computeQueue; }

  /**
   * @brief Get the presentation queue
   * @return VkQueue handle for presentation operations
   */
  VkQueue getPresentQueue() const { return m_presentQueue; }

  /**
   * @brief Create command pool for compute operations
   *
   * Creates a command pool suitable for allocating command buffers
   * for compute operations. Uses the compute queue family.
   *
   * @return VkCommandPool handle
   * @throws std::runtime_error If command pool creation fails
   */
  VkCommandPool createComputeCommandPool();

  /**
   * @brief Create command pool for graphics operations
   *
   * Creates a command pool suitable for allocating command buffers
   * for graphics operations. Uses the graphics queue family.
   *
   * @return VkCommandPool handle
   * @throws std::runtime_error If command pool creation fails
   */
  VkCommandPool createGraphicsCommandPool();

  /**
   * @brief Create command pool for transfer operations
   *
   * Creates a command pool suitable for allocating command buffers
   * for transfer operations. Uses the transfer or graphics queue family.
   *
   * @return VkCommandPool handle
   * @throws std::runtime_error If command pool creation fails
   */
  VkCommandPool createTransferCommandPool();

  // TODO(Phase 2): Add memory allocation utilities
  // Details: Helper functions for buffer/image creation and memory binding
  // Priority: High
  // Dependencies: Device creation complete

  // TODO(Phase 5): Add device feature queries
  // Details: Runtime feature detection for optimization
  // Priority: Low
  // Dependencies: Core functionality stable

private:
  /**
   * @brief Create the Vulkan instance
   *
   * Creates a VkInstance with appropriate extensions and validation layers.
   * Validation layers are enabled in debug builds for development assistance.
   *
   * @throws std::runtime_error If instance creation fails
   */
  void createInstance();

  /**
   * @brief Set up debug messenger for validation layers
   *
   * Creates a debug messenger to receive validation layer messages.
   * Only active in debug builds when validation layers are enabled.
   *
   * @throws std::runtime_error If debug messenger creation fails
   */
  void setupDebugMessenger();

  /**
   * @brief Create window surface
   *
   * Creates a VkSurfaceKHR for presenting rendered images to the window.
   * Uses the window manager to create a platform-appropriate surface.
   *
   * @param windowManager Window manager for surface creation
   * @throws std::runtime_error If surface creation fails
   */
  void createSurface(const WindowManager &windowManager);

  /**
   * @brief Select the best physical device
   *
   * Enumerates available physical devices, scores them based on our
   * requirements, and selects the best one for fractal generation.
   *
   * @throws std::runtime_error If no suitable device found
   */
  void pickPhysicalDevice();

  /**
   * @brief Create logical device and queues
   *
   * Creates a VkDevice with the necessary queues for graphics, compute,
   * and presentation operations. Enables required device features.
   *
   * @throws std::runtime_error If device creation fails
   */
  void createLogicalDevice();

  /**
   * @brief Score a physical device for suitability
   *
   * Calculates a score for a physical device based on:
   * - Device type (discrete GPU preferred)
   * - Queue family capabilities
   * - Available memory
   * - Supported features
   *
   * @param device Physical device to score
   * @return PhysicalDeviceInfo with calculated score
   */
  PhysicalDeviceInfo scorePhysicalDevice(VkPhysicalDevice device);

  /**
   * @brief Find queue families for a physical device
   *
   * Examines the queue families available on a physical device and
   * identifies which ones support the operations we need.
   *
   * @param device Physical device to examine
   * @return QueueFamilyIndices structure with family indices
   */
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

  /**
   * @brief Check if a device supports required extensions
   *
   * Verifies that a physical device supports all the Vulkan extensions
   * we need for our fractal generator.
   *
   * @param device Physical device to check
   * @return true if all required extensions supported, false otherwise
   */
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);

  /**
   * @brief Get required instance extensions
   *
   * Returns a list of Vulkan instance extensions required for our application.
   * Includes GLFW extensions and debug extensions (debug builds only).
   *
   * @return Vector of required extension names
   */
  std::vector<const char *> getRequiredExtensions();

  /**
   * @brief Check validation layer support
   *
   * Verifies that all requested validation layers are available.
   * Only relevant for debug builds.
   *
   * @return true if all validation layers available, false otherwise
   */
  bool checkValidationLayerSupport();

  /**
   * @brief Debug messenger callback function
   *
   * Static callback function for validation layer messages.
   * Logs validation errors, warnings, and info messages.
   *
   * @param messageSeverity Severity level of the message
   * @param messageType Type of message (validation, performance, etc.)
   * @param pCallbackData Message details
   * @param pUserData User data pointer (unused)
   * @return VK_FALSE (don't abort)
   */
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData);

  // Vulkan object handles

  VkInstance m_instance;                     ///< Vulkan instance
  VkDebugUtilsMessengerEXT m_debugMessenger; ///< Debug messenger (debug builds)
  VkSurfaceKHR m_surface;                    ///< Window surface
  VkPhysicalDevice m_physicalDevice;         ///< Selected physical device
  VkDevice m_device;                         ///< Logical device

  // Queue handles
  VkQueue m_graphicsQueue; ///< Graphics queue
  VkQueue m_computeQueue;  ///< Compute queue
  VkQueue m_presentQueue;  ///< Presentation queue

  // Queue family information
  QueueFamilyIndices m_queueFamilies; ///< Queue family indices

  // Configuration
  const std::vector<const char *> m_validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  const std::vector<const char *> m_deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
      // TODO(Phase 2): Add compute-specific extensions if needed
      // TODO(Phase 5): Add ray tracing extensions for advanced fractals
  };

#ifdef VK_ENABLE_VALIDATION_LAYERS
  static constexpr bool m_enableValidationLayers = true;
#else
  static constexpr bool m_enableValidationLayers = false;
#endif
};

/**
 * Implementation Notes:
 *
 * 1. Vulkan Initialization Process:
 *    - Instance creation with extensions and validation layers
 *    - Physical device selection with comprehensive scoring
 *    - Logical device creation with required queues
 *    - Proper error checking at each step
 *
 * 2. Queue Family Management:
 *    - Separate queues for graphics, compute, and presentation
 *    - Fallback to unified queue if separate queues unavailable
 *    - Future optimization: transfer queue for memory operations
 *
 * 3. Resource Management:
 *    - RAII pattern for automatic cleanup
 *    - Proper destruction order (device before instance)
 *    - Exception safety throughout initialization
 *
 * 4. Validation and Debugging:
 *    - Validation layers in debug builds only
 *    - Comprehensive debug message handling
 *    - Clear error messages for common issues
 *
 * 5. Extensibility:
 *    - Designed for easy addition of features in future phases
 *    - Clean separation between initialization and usage
 *    - Prepared for compute pipeline and advanced features
 */
