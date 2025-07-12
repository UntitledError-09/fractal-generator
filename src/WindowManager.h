/**
 * @file WindowManager.h
 * @brief GLFW window management and Vulkan surface creation
 *
 * This class encapsulates all windowing system interactions using GLFW,
 * including window creation, event handling, and Vulkan surface creation.
 * It provides a clean interface between the application and the windowing
 * system.
 *
 * Phase 1 Focus:
 * - Basic GLFW window creation and management
 * - Event processing integration
 * - Vulkan surface creation for future rendering
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 1
 */

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <string>

/**
 * @class WindowManager
 * @brief Manages GLFW window and Vulkan surface creation
 *
 * This class provides a clean abstraction over GLFW windowing functionality
 * and integrates with Vulkan for surface creation. It follows RAII principles
 * for automatic resource management.
 *
 * Key Responsibilities:
 * - GLFW initialization and termination
 * - Window creation with appropriate properties
 * - Event processing and callback management
 * - Vulkan surface creation and management
 *
 * Design Notes:
 * - Uses RAII for automatic GLFW cleanup
 * - Exception-safe initialization
 * - Extensible event handling for future GUI integration
 */
class WindowManager {
public:
  /**
   * @brief Create and initialize a GLFW window
   *
   * Initializes GLFW, creates a window with the specified properties,
   * and sets up basic event handling. The window is configured for
   * Vulkan rendering (no OpenGL context).
   *
   * @param width Initial window width in pixels
   * @param height Initial window height in pixels
   * @param title Window title string
   *
   * @throws std::runtime_error If GLFW initialization or window creation fails
   */
  WindowManager(int width, int height, const std::string &title);

  /**
   * @brief Destructor - cleanup GLFW resources
   *
   * Destroys the window and terminates GLFW. Ensures proper cleanup
   * order and handles any errors gracefully.
   */
  ~WindowManager();

  // Disable copy construction and assignment
  // GLFW windows are not copyable
  WindowManager(const WindowManager &) = delete;
  WindowManager &operator=(const WindowManager &) = delete;

  // TODO(Phase 4): Implement move semantics if needed
  // Details: May be useful for window management flexibility
  // Priority: Low
  // Dependencies: Clear use case

  /**
   * @brief Process all pending window events
   *
   * Calls glfwPollEvents() to process window system events including:
   * - Window close requests
   * - Keyboard input
   * - Mouse input
   * - Window resize events
   *
   * This should be called once per frame in the main loop.
   */
  void pollEvents();

  /**
   * @brief Check if the window should be closed
   *
   * Returns true if the user has requested to close the window
   * (e.g., clicking the X button, pressing Alt+F4, etc.).
   *
   * @return true if the window should close, false otherwise
   */
  bool shouldClose() const;

  /**
   * @brief Get the GLFW window handle
   *
   * Provides access to the underlying GLFW window for operations
   * that require direct GLFW access, such as Vulkan surface creation.
   *
   * @return GLFW window handle
   */
  GLFWwindow *getWindow() const { return m_window; }

  /**
   * @brief Create a Vulkan surface for this window
   *
   * Creates a VkSurfaceKHR that Vulkan can use for rendering to this window.
   * This integrates the window system with Vulkan's rendering pipeline.
   *
   * @param instance Vulkan instance to create the surface for
   * @return VkSurfaceKHR handle for the window surface
   *
   * @throws std::runtime_error If surface creation fails
   */
  VkSurfaceKHR createVulkanSurface(VkInstance instance) const;

  /**
   * @brief Get the current window size
   *
   * Retrieves the current window dimensions, which may have changed
   * due to user resizing or system events.
   *
   * @param width Output parameter for window width
   * @param height Output parameter for window height
   */
  void getWindowSize(int &width, int &height) const;

  /**
   * @brief Get the framebuffer size
   *
   * Retrieves the size of the framebuffer in pixels. This may differ
   * from window size on high-DPI displays.
   *
   * @param width Output parameter for framebuffer width
   * @param height Output parameter for framebuffer height
   */
  void getFramebufferSize(int &width, int &height) const;

  /**
   * @brief Set window resize callback
   *
   * Registers a callback function to be called when the window is resized.
   * This will be used in future phases for handling viewport changes.
   *
   * @param callback Function to call on window resize
   */
  void setResizeCallback(std::function<void(int, int)> callback);

  /**
   * @brief Input handling methods - Phase 3 implementation
   *
   * These methods provide comprehensive input handling for fractal navigation,
   * including keyboard shortcuts, mouse interaction, and parameter control.
   */

  /**
   * @brief Check if a key is currently pressed
   *
   * @param key GLFW key code (e.g., GLFW_KEY_W, GLFW_KEY_ESCAPE)
   * @return true if key is pressed, false otherwise
   */
  bool isKeyPressed(int key) const;

  /**
   * @brief Check if a mouse button is currently pressed
   *
   * @param button GLFW mouse button code (e.g., GLFW_MOUSE_BUTTON_LEFT)
   * @return true if button is pressed, false otherwise
   */
  bool isMouseButtonPressed(int button) const;

  /**
   * @brief Get current mouse cursor position
   *
   * @param xpos Output parameter for X coordinate
   * @param ypos Output parameter for Y coordinate
   */
  void getMousePosition(double &xpos, double &ypos) const;

  /**
   * @brief Set key callback for advanced input handling
   *
   * @param callback Function to call on key events
   */
  void setKeyCallback(std::function<void(int, int, int, int)> callback);

  /**
   * @brief Set mouse button callback
   *
   * @param callback Function to call on mouse button events
   */
  void setMouseButtonCallback(std::function<void(int, int, int)> callback);

  /**
   * @brief Set mouse position callback for continuous tracking
   *
   * @param callback Function to call on mouse movement
   */
  void setMousePositionCallback(std::function<void(double, double)> callback);

  /**
   * @brief Set scroll callback for zoom controls
   *
   * @param callback Function to call on scroll events
   */
  void setScrollCallback(std::function<void(double, double)> callback);

  /**
   * @brief Fullscreen support - Phase 3 implementation
   *
   * Methods for toggling between windowed and fullscreen modes
   * for immersive fractal exploration.
   */

  /**
   * @brief Check if window is currently in fullscreen mode
   *
   * @return true if fullscreen, false if windowed
   */
  bool isFullscreen() const;

  /**
   * @brief Toggle between windowed and fullscreen modes
   *
   * Switches the window between windowed and fullscreen modes,
   * preserving window state and handling monitor selection.
   */
  void toggleFullscreen();

  /**
   * @brief Enter fullscreen mode on specified monitor
   *
   * @param monitor GLFW monitor handle (nullptr for primary monitor)
   */
  void enterFullscreen(GLFWmonitor *monitor = nullptr);

  /**
   * @brief Exit fullscreen mode and return to windowed
   */
  void exitFullscreen();

  // TODO(Phase 5): Add multi-monitor support
  // Details: Handle multiple displays, monitor enumeration
  // Priority: Low
  // Dependencies: Core functionality stable

private:
  /**
   * @brief Initialize GLFW library
   *
   * Performs GLFW initialization and sets up error callbacks.
   * Called from constructor.
   *
   * @throws std::runtime_error If GLFW initialization fails
   */
  void initializeGLFW();

  /**
   * @brief Create the GLFW window
   *
   * Creates a GLFW window with Vulkan-compatible settings.
   * No OpenGL context is created since we're using Vulkan.
   *
   * @param width Window width in pixels
   * @param height Window height in pixels
   * @param title Window title string
   *
   * @throws std::runtime_error If window creation fails
   */
  void createWindow(int width, int height, const std::string &title);

  /**
   * @brief Set up window callbacks
   *
   * Registers callback functions for window events.
   * These callbacks will be expanded in future phases.
   */
  void setupCallbacks();

  /**
   * @brief GLFW error callback function
   *
   * Static callback function for GLFW error reporting.
   * Logs errors to help with debugging.
   *
   * @param error GLFW error code
   * @param description Human-readable error description
   */
  static void glfwErrorCallback(int error, const char *description);

  /**
   * @brief Window resize callback function
   *
   * Static callback function for window resize events.
   * Forwards the call to the instance callback if registered.
   *
   * @param window GLFW window that was resized
   * @param width New window width
   * @param height New window height
   */
  static void glfwResizeCallback(GLFWwindow *window, int width, int height);

  /**
   * @brief Input handling static callbacks - Phase 3
   */
  static void glfwKeyCallback(GLFWwindow *window, int key, int scancode,
                              int action, int mods);
  static void glfwMouseButtonCallback(GLFWwindow *window, int button,
                                      int action, int mods);
  static void glfwMousePositionCallback(GLFWwindow *window, double xpos,
                                        double ypos);
  static void glfwScrollCallback(GLFWwindow *window, double xoffset,
                                 double yoffset);

  // Member variables

  /**
   * @brief GLFW window handle
   *
   * The main window handle used for all GLFW operations.
   */
  GLFWwindow *m_window;

  /**
   * @brief Current window width
   *
   * Cached window width, updated on resize events.
   */
  int m_width;

  /**
   * @brief Current window height
   *
   * Cached window height, updated on resize events.
   */
  int m_height;

  /**
   * @brief Window title string
   *
   * Stored for potential future use (window title updates, etc.).
   */
  std::string m_title;

  /**
   * @brief User resize callback function
   *
   * Optional callback function called when the window is resized.
   * Will be used for viewport updates in future phases.
   */
  std::function<void(int, int)> m_resizeCallback;

  /**
   * @brief Input handling callbacks - Phase 3
   */
  std::function<void(int, int, int, int)> m_keyCallback;
  std::function<void(int, int, int)> m_mouseButtonCallback;
  std::function<void(double, double)> m_mousePositionCallback;
  std::function<void(double, double)> m_scrollCallback;

  /**
   * @brief Fullscreen state tracking - Phase 3
   */
  bool m_isFullscreen;
  int m_windowedWidth;
  int m_windowedHeight;
  int m_windowedPosX;
  int m_windowedPosY;

  /**
   * @brief GLFW initialization flag
   *
   * Tracks whether GLFW was successfully initialized so we know
   * whether to call glfwTerminate() in the destructor.
   */
  bool m_glfwInitialized;
};

/**
 * Implementation Notes:
 *
 * 1. GLFW Integration:
 *    - Uses GLFW_INCLUDE_VULKAN for automatic Vulkan header inclusion
 *    - No OpenGL context created (windowHint GLFW_CLIENT_API, GLFW_NO_API)
 *    - Proper error handling and callback setup
 *
 * 2. Resource Management:
 *    - RAII pattern ensures proper cleanup
 *    - Exception safety in constructor
 *    - Graceful error handling in destructor
 *
 * 3. Vulkan Integration:
 *    - Surface creation method for Vulkan rendering
 *    - Framebuffer size handling for high-DPI displays
 *    - Window resize handling for swapchain recreation
 *
 * 4. Extensibility:
 *    - Callback system ready for complex input handling
 *    - Clean interface for future GUI integration
 *    - Prepared for fullscreen and multi-monitor support
 *
 * 5. Phase 1 Scope:
 *    - Basic window creation and event processing
 *    - Foundation for future input and rendering features
 *    - Comprehensive error handling and logging
 */
