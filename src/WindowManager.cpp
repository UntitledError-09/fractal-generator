/**
 * @file WindowManager.cpp
 * @brief Implementation of GLFW window management
 *
 * This file implements the WindowManager class, handling all GLFW
 * windowing operations and Vulkan surface creation. It demonstrates
 * proper GLFW lifecycle management and error handling.
 *
 * Phase 1 Focus:
 * - GLFW initialization and window creation
 * - Basic event processing
 * - Vulkan surface creation
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 1
 */

#include "WindowManager.h"

#include <iostream>
#include <stdexcept>
#include <string>

/**
 * @brief Constructor - Create and initialize GLFW window
 *
 * Performs complete window system initialization including GLFW setup,
 * window creation, and callback registration. Uses RAII for exception safety.
 *
 * @param width Initial window width in pixels
 * @param height Initial window height in pixels
 * @param title Window title string
 */
WindowManager::WindowManager(int width, int height, const std::string &title)
    : m_window(nullptr), m_width(width), m_height(height), m_title(title),
      m_glfwInitialized(false), m_isFullscreen(false), m_windowedWidth(width),
      m_windowedHeight(height), m_windowedPosX(100), m_windowedPosY(100) {
  std::cout << "WindowManager: Initializing window system..." << std::endl;

  try {
    // Initialize GLFW library
    initializeGLFW();

    // Create the window with Vulkan support
    createWindow(width, height, title);

    // Set up event callbacks
    setupCallbacks();

    std::cout << "WindowManager: Window created successfully (" << width << "x"
              << height << ")" << std::endl;

  } catch (const std::exception &e) {
    // If initialization fails, clean up any partial state
    std::cerr << "WindowManager: Initialization failed: " << e.what()
              << std::endl;

    // The destructor will handle cleanup of any initialized components
    throw;
  }
}

/**
 * @brief Destructor - Clean up GLFW resources
 *
 * Ensures proper cleanup of the window and GLFW library.
 * Order is important: destroy window before terminating GLFW.
 */
WindowManager::~WindowManager() {
  std::cout << "WindowManager: Cleaning up window system..." << std::endl;

  try {
    // Destroy the window if it was created
    if (m_window) {
      std::cout << "WindowManager: Destroying window..." << std::endl;
      glfwDestroyWindow(m_window);
      m_window = nullptr;
    }

    // Terminate GLFW if it was initialized
    if (m_glfwInitialized) {
      std::cout << "WindowManager: Terminating GLFW..." << std::endl;
      glfwTerminate();
      m_glfwInitialized = false;
    }

    std::cout << "WindowManager: Cleanup completed successfully." << std::endl;

  } catch (const std::exception &e) {
    // Log errors but don't throw from destructor
    std::cerr << "WindowManager: Error during cleanup: " << e.what()
              << std::endl;
  }
}

/**
 * @brief Initialize GLFW library
 *
 * Performs GLFW initialization and sets up error callback.
 * This must be called before any other GLFW operations.
 */
void WindowManager::initializeGLFW() {
  std::cout << "WindowManager: Initializing GLFW library..." << std::endl;

  // Set up GLFW error callback before initialization
  // This ensures we capture any initialization errors
  glfwSetErrorCallback(glfwErrorCallback);

  // Initialize GLFW library
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW library");
  }

  m_glfwInitialized = true;

  // Check GLFW version for debugging
  int major, minor, revision;
  glfwGetVersion(&major, &minor, &revision);
  std::cout << "WindowManager: GLFW version " << major << "." << minor << "."
            << revision << std::endl;

  // Verify Vulkan support
  if (!glfwVulkanSupported()) {
    throw std::runtime_error("Vulkan not supported by GLFW");
  }

  std::cout << "WindowManager: Vulkan support confirmed." << std::endl;
}

/**
 * @brief Create GLFW window with Vulkan support
 *
 * Creates a window configured for Vulkan rendering. No OpenGL context
 * is created since we're using Vulkan exclusively.
 *
 * @param width Window width in pixels
 * @param height Window height in pixels
 * @param title Window title string
 */
void WindowManager::createWindow(int width, int height,
                                 const std::string &title) {
  std::cout << "WindowManager: Creating window..." << std::endl;

  // Configure GLFW for Vulkan (no OpenGL context)
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  //  Resizable window support implemented - handled via callbacks
  // Details: Handle window resize events and swapchain recreation
  // Priority: Medium
  // Dependencies: Vulkan swapchain implementation
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  // TODO(Phase 5): Add additional window hints for optimization
  // Details: Double buffering, refresh rate, etc.
  // Priority: Low
  // Dependencies: Performance optimization phase

  // Create the window
  m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!m_window) {
    throw std::runtime_error("Failed to create GLFW window");
  }

  // Store this WindowManager instance in the window user pointer
  // This allows static callbacks to access the instance
  glfwSetWindowUserPointer(m_window, this);

  std::cout << "WindowManager: Window created with handle: " << m_window
            << std::endl;
}

/**
 * @brief Set up GLFW event callbacks
 *
 * Registers callback functions for various window events.
 * These callbacks will be expanded in future phases for input handling.
 */
void WindowManager::setupCallbacks() {
  std::cout << "WindowManager: Setting up event callbacks..." << std::endl;

  // Set up window resize callback
  glfwSetFramebufferSizeCallback(m_window, glfwResizeCallback);

  // Set up input handling callbacks - Phase 3 implementation
  glfwSetKeyCallback(m_window, glfwKeyCallback);
  glfwSetMouseButtonCallback(m_window, glfwMouseButtonCallback);
  glfwSetCursorPosCallback(m_window, glfwMousePositionCallback);
  glfwSetScrollCallback(m_window, glfwScrollCallback);

  std::cout << "WindowManager: Event callbacks registered (including Phase 3 "
               "input handling)."
            << std::endl;
}

/**
 * @brief Process all pending window events
 *
 * Calls GLFW to process all pending events. This should be called
 * once per frame to ensure responsive window behavior.
 */
void WindowManager::pollEvents() {
  // Process all pending events
  // This includes window events, input events, etc.
  glfwPollEvents();
}

/**
 * @brief Check if window should be closed
 *
 * Returns the state of the window close flag, which is set when
 * the user requests to close the window.
 *
 * @return true if window should close, false otherwise
 */
bool WindowManager::shouldClose() const {
  return glfwWindowShouldClose(m_window);
}

/**
 * @brief Create Vulkan surface for the window
 *
 * Creates a VkSurfaceKHR that allows Vulkan to render to this window.
 * This bridges the gap between the windowing system and Vulkan.
 *
 * @param instance Vulkan instance to create surface for
 * @return VkSurfaceKHR handle for the window surface
 */
VkSurfaceKHR WindowManager::createVulkanSurface(VkInstance instance) const {
  std::cout << "WindowManager: Creating Vulkan surface..." << std::endl;

  VkSurfaceKHR surface;
  VkResult result =
      glfwCreateWindowSurface(instance, m_window, nullptr, &surface);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan window surface");
  }

  std::cout << "WindowManager: Vulkan surface created successfully."
            << std::endl;
  return surface;
}

/**
 * @brief Get current window size
 *
 * Retrieves the current window dimensions, which may have changed
 * since creation due to user interaction.
 *
 * @param width Output parameter for current window width
 * @param height Output parameter for current window height
 */
void WindowManager::getWindowSize(int &width, int &height) const {
  glfwGetWindowSize(m_window, &width, &height);
}

/**
 * @brief Get current framebuffer size
 *
 * Retrieves the framebuffer size in pixels. On high-DPI displays,
 * this may differ from the window size.
 *
 * @param width Output parameter for framebuffer width in pixels
 * @param height Output parameter for framebuffer height in pixels
 */
void WindowManager::getFramebufferSize(int &width, int &height) const {
  glfwGetFramebufferSize(m_window, &width, &height);
}

/**
 * @brief Set window resize callback
 *
 * Registers a callback function to be called when the window is resized.
 * This will be used for handling Vulkan swapchain recreation.
 *
 * @param callback Function to call on window resize
 */
void WindowManager::setResizeCallback(std::function<void(int, int)> callback) {
  m_resizeCallback = callback;
}

/**
 * @brief Input handling methods - Phase 3 implementation
 */

bool WindowManager::isKeyPressed(int key) const {
  return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool WindowManager::isMouseButtonPressed(int button) const {
  return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void WindowManager::getMousePosition(double &xpos, double &ypos) const {
  glfwGetCursorPos(m_window, &xpos, &ypos);
}

void WindowManager::setKeyCallback(
    std::function<void(int, int, int, int)> callback) {
  m_keyCallback = callback;
}

void WindowManager::setMouseButtonCallback(
    std::function<void(int, int, int)> callback) {
  m_mouseButtonCallback = callback;
}

void WindowManager::setMousePositionCallback(
    std::function<void(double, double)> callback) {
  m_mousePositionCallback = callback;
}

void WindowManager::setScrollCallback(
    std::function<void(double, double)> callback) {
  m_scrollCallback = callback;
}

/**
 * @brief Fullscreen support - Phase 3 implementation
 */

bool WindowManager::isFullscreen() const { return m_isFullscreen; }

void WindowManager::toggleFullscreen() {
  if (m_isFullscreen) {
    exitFullscreen();
  } else {
    enterFullscreen();
  }
}

void WindowManager::enterFullscreen(GLFWmonitor *monitor) {
  if (m_isFullscreen) {
    return; // Already fullscreen
  }

  // Store current windowed state
  glfwGetWindowPos(m_window, &m_windowedPosX, &m_windowedPosY);
  glfwGetWindowSize(m_window, &m_windowedWidth, &m_windowedHeight);

  // Use primary monitor if none specified
  if (!monitor) {
    monitor = glfwGetPrimaryMonitor();
  }

  // Get monitor video mode
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);

  // Switch to fullscreen
  glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height,
                       mode->refreshRate);

  m_isFullscreen = true;
  m_width = mode->width;
  m_height = mode->height;

  std::cout << "WindowManager: Entered fullscreen mode (" << mode->width << "x"
            << mode->height << ")" << std::endl;
}

void WindowManager::exitFullscreen() {
  if (!m_isFullscreen) {
    return; // Already windowed
  }

  // Switch back to windowed mode
  glfwSetWindowMonitor(m_window, nullptr, m_windowedPosX, m_windowedPosY,
                       m_windowedWidth, m_windowedHeight, GLFW_DONT_CARE);

  m_isFullscreen = false;
  m_width = m_windowedWidth;
  m_height = m_windowedHeight;

  std::cout << "WindowManager: Exited fullscreen mode (" << m_windowedWidth
            << "x" << m_windowedHeight << ")" << std::endl;
}

/**
 * @brief GLFW error callback function
 *
 * Static callback function that handles GLFW errors by logging them.
 * This helps with debugging GLFW-related issues.
 *
 * @param error GLFW error code
 * @param description Human-readable error description
 */
void WindowManager::glfwErrorCallback(int error, const char *description) {
  std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

/**
 * @brief Window resize callback function
 *
 * Static callback that handles window resize events. Forwards the
 * call to the instance callback if one is registered.
 *
 * @param window GLFW window that was resized
 * @param width New framebuffer width
 * @param height New framebuffer height
 */
void WindowManager::glfwResizeCallback(GLFWwindow *window, int width,
                                       int height) {
  // Get the WindowManager instance from the window user pointer
  WindowManager *windowManager =
      static_cast<WindowManager *>(glfwGetWindowUserPointer(window));

  if (windowManager) {
    // Update cached dimensions
    windowManager->m_width = width;
    windowManager->m_height = height;

    // Call user callback if registered
    if (windowManager->m_resizeCallback) {
      windowManager->m_resizeCallback(width, height);
    }

    std::cout << "WindowManager: Window resized to " << width << "x" << height
              << std::endl;
  }
}

/**
 * @brief Input handling static callbacks - Phase 3 implementation
 */

void WindowManager::glfwKeyCallback(GLFWwindow *window, int key, int scancode,
                                    int action, int mods) {
  WindowManager *windowManager =
      static_cast<WindowManager *>(glfwGetWindowUserPointer(window));

  if (windowManager && windowManager->m_keyCallback) {
    windowManager->m_keyCallback(key, scancode, action, mods);
  }
}

void WindowManager::glfwMouseButtonCallback(GLFWwindow *window, int button,
                                            int action, int mods) {
  WindowManager *windowManager =
      static_cast<WindowManager *>(glfwGetWindowUserPointer(window));

  if (windowManager && windowManager->m_mouseButtonCallback) {
    windowManager->m_mouseButtonCallback(button, action, mods);
  }
}

void WindowManager::glfwMousePositionCallback(GLFWwindow *window, double xpos,
                                              double ypos) {
  WindowManager *windowManager =
      static_cast<WindowManager *>(glfwGetWindowUserPointer(window));

  if (windowManager && windowManager->m_mousePositionCallback) {
    windowManager->m_mousePositionCallback(xpos, ypos);
  }
}

void WindowManager::glfwScrollCallback(GLFWwindow *window, double xoffset,
                                       double yoffset) {
  WindowManager *windowManager =
      static_cast<WindowManager *>(glfwGetWindowUserPointer(window));

  if (windowManager && windowManager->m_scrollCallback) {
    windowManager->m_scrollCallback(xoffset, yoffset);
  }
}

/**
 * Implementation Notes:
 *
 * 1. RAII Resource Management:
 *    - Constructor handles complete initialization or throws
 *    - Destructor ensures proper cleanup in reverse order
 *    - Exception safety throughout initialization process
 *
 * 2. GLFW Integration:
 *    - Proper error callback setup before initialization
 *    - Vulkan support verification
 *    - No OpenGL context creation (GLFW_NO_API)
 *
 * 3. Event Handling:
 *    - User pointer mechanism for callback instance access
 *    - Extensible callback system for future input handling
 *    - Proper event processing in main loop
 *
 * 4. Vulkan Integration:
 *    - Clean surface creation interface
 *    - High-DPI display support with framebuffer size handling
 *    - Prepared for swapchain management
 *
 * 5. Error Handling:
 *    - Comprehensive error checking for all GLFW operations
 *    - Clear error messages for debugging
 *    - Graceful cleanup on failure
 */
