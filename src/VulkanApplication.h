/**
 * @file VulkanApplication.h
 * @brief Main application class for the Vulkan fractal generator
 * 
 * This class encapsulates the entire Vulkan application lifecycle, from
 * initialization through rendering to cleanup. It serves as the central
 * coordinator for all subsystems including Vulkan setup, window management,
 * and future rendering pipelines.
 * 
 * Phase 1 Focus: ✅ COMPLETE
 * - Application lifecycle management
 * - Integration with VulkanSetup and WindowManager
 * - Basic error handling and resource cleanup
 * 
 * Phase 2 Focus: ✅ COMPLETE  
 * - Compute pipeline integration
 * - GPU fractal computation
 * - Memory management and buffer allocation
 * 
 * Phase 3 Focus: ✅ COMPLETE
 * - Graphics pipeline implementation
 * - Swapchain management and presentation
 * - Fullscreen quad rendering with test pattern
 * 
 * Phase 4 Focus: 🎯 NEXT
 * - Fractal integration (connect compute output to graphics input)
 * - Texture creation and data transfer
 * - Real fractal visualization
 * 
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 3 Complete - Graphics Rendering Functional
 */

#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <vector>

// Forward declarations to minimize header dependencies
// This follows good C++ practices for faster compilation
class VulkanSetup;
class WindowManager;
class ShaderManager;
class MemoryManager;
class ComputePipeline;
class SwapchainManager;
class GraphicsPipeline;
class TextureManager;

/**
 * @class VulkanApplication
 * @brief Main application controller for the fractal generator
 * 
 * This class follows the RAII (Resource Acquisition Is Initialization) pattern
 * to ensure proper cleanup of all Vulkan resources. It coordinates between
 * the various subsystems and provides a clean interface for the main() function.
 * 
 * Design Principles:
 * - Single Responsibility: Coordinates subsystems, doesn't implement details
 * - RAII: Automatic resource management through constructor/destructor
 * - Exception Safety: Strong exception safety guarantee
 * - Phase-based Development: Functionality added incrementally
 */
class VulkanApplication {
public:
    /**
     * @brief Construct and initialize the Vulkan application
     * 
     * Performs all necessary initialization including:
     * - GLFW window creation
     * - Vulkan instance and device setup
     * - Initial resource allocation
     * 
     * Phase 1 Implementation:
     * - Basic window creation
     * - Vulkan instance with validation layers
     * - Physical and logical device selection
     * 
     * @throws std::runtime_error If initialization fails
     * @throws VulkanException If Vulkan-specific errors occur
     */
    VulkanApplication();
    
    /**
     * @brief Destructor - cleanup all resources
     * 
     * Ensures proper cleanup order:
     * 1. Vulkan resources (reverse creation order)
     * 2. Window system cleanup
     * 3. GLFW termination
     * 
     * Note: Destructor should not throw exceptions
     */
    ~VulkanApplication();
    
    // Disable copy construction and assignment
    // Vulkan resources are not copyable, and moving would be complex
    VulkanApplication(const VulkanApplication&) = delete;
    VulkanApplication& operator=(const VulkanApplication&) = delete;
    
    // TODO(Phase 4): Implement move semantics if needed
    // Details: May be useful for factory functions or container storage
    // Priority: Low
    // Dependencies: Clear use case identification
    
    /**
     * @brief Run the main application loop
     * 
     * Handles the main application lifecycle:
     * - Event processing (window, input)
     * - Frame rendering
     * - Performance monitoring
     * - Graceful shutdown handling
     * 
     * Phase 1 Implementation:
     * - Basic event loop with GLFW
     * - Simple "application running" feedback
     * - Proper event handling and cleanup
     * 
     * Future Phases:
     * - Fractal computation and rendering (Phase 2-3)
     * - GUI integration (Phase 3)
     * - Performance profiling (Phase 5)
     * 
     * @throws std::runtime_error If runtime errors occur
     */
    void run();
    
    /**
     * @brief Get the window title string
     * 
     * Provides a descriptive title for the application window including
     * version information and current development phase.
     * 
     * @return String containing the window title
     */
    static std::string getWindowTitle();
    
private:
    /**
     * @brief Initialize all application subsystems
     * 
     * Coordinates the initialization of:
     * - Window management system
     * - Vulkan setup (instance, device, etc.)
     * - Basic resource allocation
     * 
     * Called from constructor. Separated for clarity and testing.
     * 
     * @throws std::runtime_error If any subsystem fails to initialize
     */
    void initializeSubsystems();
    
    /**
     * @brief Process window and input events
     * 
     * Handles:
     * - Window close events
     * - Keyboard input
     * - Mouse input
     * - Window resize events
     * 
     * Phase 1: Basic event polling and window close detection
     * Future: Input handling for fractal navigation
     */
    void processEvents();
    
    /**
     * @brief Render a single frame
     * 
     * Phase 1: Placeholder implementation
     * Phase 2: Basic compute dispatch
     * Phase 3: Full rendering pipeline
     * 
     * Future implementation will include:
     * - Parameter updates
     * - Compute shader dispatch
     * - Graphics rendering
     * - Frame presentation
     */
    void renderFrame();
    
    /**
     * @brief Update application state
     * 
     * Handles per-frame updates that don't involve rendering:
     * - Animation timing
     * - Parameter interpolation
     * - Performance metrics collection
     * 
     * Phase 1: Minimal implementation
     * Future: Complex state management
     * 
     * @param deltaTime Time elapsed since last frame (seconds)
     */
    void updateApplication(double deltaTime);
    
    // Subsystem managers - using unique_ptr for forward declaration compatibility
    // This allows us to keep implementation details in the .cpp file
    
    /**
     * @brief Window management subsystem
     * 
     * Handles GLFW window creation, event processing, and Vulkan surface creation.
     * Encapsulates all windowing system interactions.
     */
    std::unique_ptr<WindowManager> m_windowManager;
    
    /**
     * @brief Vulkan setup and management subsystem
     * 
     * Handles Vulkan instance creation, device selection, and basic resource
     * management. Provides the foundation for all Vulkan operations.
     */
    std::unique_ptr<VulkanSetup> m_vulkanSetup;
    
    /**
     * @brief Shader compilation and management subsystem
     * 
     * Handles GLSL to SPIR-V compilation, shader module creation, and
     * shader resource lifecycle management.
     */
    std::shared_ptr<ShaderManager> m_shaderManager;
    
    /**
     * @brief Memory allocation and buffer management subsystem
     * 
     * Provides high-level utilities for Vulkan memory management,
     * buffer creation, and data transfer operations.
     */
    std::shared_ptr<MemoryManager> m_memoryManager;
    
    /**
     * @brief Compute pipeline management for fractal generation
     * 
     * Manages compute pipelines, descriptor sets, and command buffers
     * for GPU-accelerated fractal computation.
     */
    std::shared_ptr<ComputePipeline> m_computePipeline;
    
    /**
     * @brief Swapchain management for window presentation
     * 
     * Handles swapchain creation, image acquisition, and presentation
     * for displaying rendered images to the window.
     */
    std::shared_ptr<SwapchainManager> m_swapchainManager;
    
    /**
     * @brief Graphics pipeline for fractal rendering
     * 
     * Manages graphics pipeline, render passes, and drawing operations
     * for rendering computed fractal data to the screen.
     */
    std::shared_ptr<GraphicsPipeline> m_graphicsPipeline;
    
    /**
     * @brief Texture management for compute-to-graphics data transfer
     * 
     * Handles texture creation, buffer-to-image copy operations,
     * and GPU memory management for fractal visualization.
     */
    std::shared_ptr<TextureManager> m_textureManager;
    
    // Application state
    
    /**
     * @brief Flag indicating whether the application should continue running
     * 
     * Set to false when the user requests shutdown or an unrecoverable
     * error occurs.
     */
    bool m_isRunning;
    
    /**
     * @brief Time of the last frame, for delta time calculation
     * 
     * Used to calculate smooth frame timing and animation updates.
     * Value is in seconds since application start.
     */
    double m_lastFrameTime;
    
    // Phase 2: Compute pipeline and fractal generation
    
    /**
     * @brief Command pool for compute operations
     * 
     * Used for allocating command buffers for compute shader dispatch.
     */
    VkCommandPool m_computeCommandPool;
    
    /**
     * @brief Command buffer for compute operations
     * 
     * Pre-allocated command buffer for fractal computation.
     */
    VkCommandBuffer m_computeCommandBuffer;
    
    // Phase 3: Graphics pipeline resources
    
    /**
     * @brief Command pool for graphics operations
     * 
     * Used for allocating command buffers for graphics rendering.
     */
    VkCommandPool m_graphicsCommandPool;
    
    /**
     * @brief Command buffers for graphics operations (one per frame)
     * 
     * Pre-allocated command buffers for graphics rendering.
     */
    std::vector<VkCommandBuffer> m_graphicsCommandBuffers;
    
    /**
     * @brief Current fractal parameters
     * 
     * Parameters for fractal computation including center, zoom, iterations, etc.
     */
    struct {
        float centerX = -0.5f;       ///< Center X coordinate in fractal space
        float centerY = 0.0f;        ///< Center Y coordinate in fractal space
        float zoom = 1.0f;           ///< Zoom level
        uint32_t maxIterations = 100; ///< Maximum iterations
        float colorScale = 1.0f;     ///< Color scaling factor
    } m_fractalParams;
    
    /**
     * @brief Fractal computation dimensions
     */
    uint32_t m_fractalWidth = 800;
    uint32_t m_fractalHeight = 600;
    
    // TODO(Phase 2): Add fractal computation state
    // Details: Parameters, zoom level, center position, iteration count
    // Priority: High
    // Dependencies: Phase 1 foundation complete
    
    // TODO(Phase 3): Add rendering state
    // Details: Pipelines, descriptor sets, command buffers
    // Priority: High  
    // Dependencies: Phase 2 compute working
    
    // TODO(Phase 4): Add threading state
    // Details: Thread pool, work queues, synchronization primitives
    // Priority: Medium
    // Dependencies: Phase 3 basic rendering working
    
    // TODO(Phase 5): Add performance monitoring state
    // Details: Frame timing, GPU profiling, memory usage tracking
    // Priority: Low
    // Dependencies: Core functionality stable
};

/**
 * Implementation Notes:
 * 
 * 1. Forward Declarations:
 *    - Reduces compilation dependencies
 *    - Allows implementation flexibility
 *    - Enables faster build times
 * 
 * 2. RAII Design:
 *    - Constructor performs all initialization
 *    - Destructor handles all cleanup
 *    - Exception safety through proper ordering
 * 
 * 3. Subsystem Coordination:
 *    - VulkanApplication doesn't implement Vulkan details
 *    - Delegates to specialized classes
 *    - Maintains clear separation of concerns
 * 
 * 4. Phase-based Development:
 *    - Interface designed for future expansion
 *    - TODO comments mark future development points
 *    - Each phase builds on previous foundation
 * 
 * 5. Error Handling:
 *    - Exceptions for unrecoverable errors
 *    - Graceful degradation where possible
 *    - Clear error messages for debugging
 */
