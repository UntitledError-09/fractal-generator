/**
 * @file GuiManager.h
 * @brief GUI management for interactive fractal parameter controls
 *
 * This class integrates Dear ImGui with the Vulkan fractal generator,
 * providing an interactive interface for fractal exploration with
 * real-time parameter adjustments.
 *
 * Phase 5 Focus:
 * - ImGui integration with Vulkan backend
 * - Multi-pane layout (fractal view + control panel)
 * - Parameter binding and change detection
 * - Render-on-change optimization
 *
 * Future Vision (VSCode-style layout):
 * - Dockable panels and flexible layouts
 * - Advanced parameter controls and presets
 * - Performance monitoring and export tools
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 5 - Interactive GUI
 */

#pragma once

#include <GLFW/glfw3.h>
#include <memory>
#include <vulkan/vulkan.h>

// Forward declarations
class VulkanSetup;
class SwapchainManager;
class GraphicsPipeline;

/**
 * @struct FractalUIParameters
 * @brief UI-bindable fractal parameters structure
 *
 * This structure mirrors the fractal computation parameters
 * but is designed for UI binding and change detection.
 */
struct FractalUIParameters {
  float centerX = -0.5f;      ///< Center X coordinate
  float centerY = 0.0f;       ///< Center Y coordinate
  float zoom = 1.0f;          ///< Zoom level
  int maxIterations = 100;    ///< Maximum iterations
  float colorScale = 1.0f;    ///< Color scaling factor
  int fractalType = 0;        ///< Fractal type (0=Mandelbrot, 1=Julia, etc.)
  int resolutionWidth = 800;  ///< Fractal resolution width
  int resolutionHeight = 600; ///< Fractal resolution height

  // UI state
  bool parametersChanged = true; ///< Flag indicating parameters have changed
  bool needsRecompute = true; ///< Flag indicating fractal needs recomputation
};

/**
 * @class GuiManager
 * @brief Manages Dear ImGui integration for fractal parameter controls
 *
 * This class provides a complete GUI system for interactive fractal
 * exploration, including parameter controls, layout management,
 * and optimization for render-on-change functionality.
 *
 * Design Philosophy:
 * - Start simple but design for VSCode-style extensibility
 * - Focus on user experience and performance
 * - Provide clean separation between UI and rendering logic
 */
class GuiManager {
public:
  /**
   * @brief Constructor
   *
   * @param vulkanSetup Shared Vulkan setup instance
   * @param swapchainManager Shared swapchain manager
   * @param graphicsPipeline Shared graphics pipeline instance
   * @param window GLFW window handle
   */
  GuiManager(std::shared_ptr<VulkanSetup> vulkanSetup,
             std::shared_ptr<SwapchainManager> swapchainManager,
             std::shared_ptr<GraphicsPipeline> graphicsPipeline,
             GLFWwindow *window);

  /**
   * @brief Destructor - cleanup ImGui resources
   */
  ~GuiManager();

  // Disable copy construction and assignment
  GuiManager(const GuiManager &) = delete;
  GuiManager &operator=(const GuiManager &) = delete;

  /**
   * @brief Initialize ImGui with Vulkan backend
   *
   * Sets up ImGui context, GLFW platform backend, and Vulkan renderer backend.
   * Creates necessary descriptor pools and resources for ImGui rendering.
   *
   * @return true if initialization successful, false otherwise
   */
  bool initialize();

  /**
   * @brief Begin new ImGui frame
   *
   * Starts a new ImGui frame and handles input processing.
   * Call this at the beginning of each render loop iteration.
   */
  void beginFrame();

  /**
   * @brief Render fractal parameter controls
   *
   * Creates the main GUI layout with fractal parameter controls.
   * Handles parameter changes and sets appropriate flags.
   *
   * @param parameters Reference to fractal parameters structure
   * @return true if any parameters changed, false otherwise
   */
  bool renderControls(FractalUIParameters &parameters);

  /**
   * @brief End ImGui frame and record render commands
   *
   * Finalizes the ImGui frame and records rendering commands
   * into the provided command buffer.
   *
   * @param commandBuffer Command buffer to record ImGui commands
   */
  void endFrame(VkCommandBuffer commandBuffer);

  /**
   * @brief Check if GUI wants to capture mouse input
   *
   * @return true if ImGui wants mouse input, false otherwise
   */
  bool wantCaptureMouse() const;

  /**
   * @brief Check if GUI wants to capture keyboard input
   *
   * @return true if ImGui wants keyboard input, false otherwise
   */
  bool wantCaptureKeyboard() const;

  /**
   * @brief Handle window resize events
   *
   * Updates ImGui display size when the window is resized.
   *
   * @param width New window width
   * @param height New window height
   */
  void handleResize(uint32_t width, uint32_t height);

  /**
   * @brief Get fractal viewport dimensions
   *
   * Calculates the available space for fractal rendering
   * after accounting for GUI panels.
   *
   * @param width Output fractal viewport width
   * @param height Output fractal viewport height
   */
  void getFractalViewport(uint32_t &width, uint32_t &height) const;

private:
  // Vulkan resources
  std::shared_ptr<VulkanSetup> m_vulkanSetup;
  std::shared_ptr<SwapchainManager> m_swapchainManager;
  std::shared_ptr<GraphicsPipeline> m_graphicsPipeline;
  GLFWwindow *m_window;

  // ImGui Vulkan resources
  VkDescriptorPool m_imguiDescriptorPool;

  // UI state
  bool m_initialized;
  uint32_t m_windowWidth;
  uint32_t m_windowHeight;

  // Layout configuration
  float m_controlPanelWidth = 300.0f; ///< Width of the control panel
  bool m_showDemoWindow = false;      ///< Show ImGui demo window
  bool m_showMetrics = false;         ///< Show performance metrics

  /**
   * @brief Create ImGui descriptor pool
   *
   * Creates a Vulkan descriptor pool specifically for ImGui rendering.
   *
   * @return true if successful, false otherwise
   */
  bool createImGuiDescriptorPool();

  /**
   * @brief Render main menu bar
   *
   * Creates the application menu bar with file operations,
   * view options, and help items.
   */
  void renderMenuBar();

  /**
   * @brief Render fractal control panel
   *
   * Creates the main fractal parameter control interface.
   *
   * @param parameters Reference to fractal parameters
   * @return true if any parameters changed
   */
  bool renderFractalControls(FractalUIParameters &parameters);

  /**
   * @brief Render performance metrics panel
   *
   * Shows frame rate, render times, and other performance data.
   */
  void renderMetricsPanel();

  /**
   * @brief Apply ImGui styling
   *
   * Sets up custom ImGui theme and styling for a modern look.
   * Future: Could support VSCode-style themes.
   */
  void setupImGuiStyle();
};

/**
 * Implementation Notes:
 *
 * 1. Phase 5.1 Implementation:
 *    - Basic split-screen layout (fractal view + control panel)
 *    - Essential parameter controls (zoom, center, iterations)
 *    - Render-on-change optimization
 *    - ImGui integration with existing Vulkan pipeline
 *
 * 2. VSCode-Style Future Vision:
 *    - The current design allows for easy extension to dockable panels
 *    - Control panel width and layout are configurable
 *    - Menu system designed for future expansion
 *    - Performance metrics foundation for advanced tools
 *
 * 3. Performance Considerations:
 *    - Separate descriptor pool for ImGui to avoid conflicts
 *    - Change detection to minimize fractal recomputation
 *    - Viewport calculation for optimal fractal resolution
 *
 * 4. User Experience:
 *    - Modern styling with custom theme support
 *    - Intuitive parameter controls with proper ranges
 *    - Real-time feedback and visual indicators
 *    - Keyboard shortcuts for power users (future)
 */
