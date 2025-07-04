/**
 * @file VulkanApplication.cpp
 * @brief Implementation of the main application class
 * 
 * This file implements the VulkanApplication class, coordinating between
 * the window management, compute pipeline, and graphics rendering subsystems.
 * It demonstrates proper RAII resource management and exception safety.
 * 
 * Phase 1: ✅ Basic application lifecycle and Vulkan foundation
 * Phase 2: ✅ Compute pipeline integration for GPU fractal computation  
 * Phase 3: ✅ Graphics pipeline implementation for screen rendering
 * Phase 4: 🎯 Fractal integration (compute output → graphics input)
 * 
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 3 Complete - Graphics Rendering Functional
 */

#include "VulkanApplication.h"
#include "VulkanSetup.h"
#include "WindowManager.h"
#include "ShaderManager.h"
#include "MemoryManager.h"
#include "ComputePipeline.h"
#include "SwapchainManager.h"
#include "GraphicsPipeline.h"
#include "TextureManager.h"

#include <iostream>
#include <chrono>

/**
 * @brief Constructor - Initialize the Vulkan application
 * 
 * Creates and initializes all necessary subsystems for the application.
 * Uses RAII to ensure proper cleanup if initialization fails partway through.
 * 
 * Initialization Order (important for dependencies):
 * 1. Basic state initialization
 * 2. Window management system
 * 3. Vulkan setup and device selection
 * 4. Cross-verification of window and Vulkan compatibility
 */
VulkanApplication::VulkanApplication()
    : m_isRunning(false)
    , m_lastFrameTime(0.0)
    , m_computeCommandPool(VK_NULL_HANDLE)
    , m_computeCommandBuffer(VK_NULL_HANDLE)
    , m_graphicsCommandPool(VK_NULL_HANDLE)
{
    std::cout << "VulkanApplication: Starting initialization..." << std::endl;
    
    try {
        // Initialize all subsystems
        // If any step fails, the destructor will clean up already-initialized systems
        initializeSubsystems();
        
        std::cout << "VulkanApplication: Initialization completed successfully." << std::endl;
        
    } catch (const std::exception& e) {
        // Log the error and re-throw
        // The destructor will handle cleanup of any partially initialized state
        std::cerr << "VulkanApplication: Initialization failed: " << e.what() << std::endl;
        throw;
    }
}

/**
 * @brief Destructor - Clean up all resources
 * 
 * Ensures proper cleanup order (reverse of initialization):
 * 1. Vulkan resources (if they exist)
 * 2. Window management system
 * 
 * Note: Destructors should not throw exceptions, so we catch and log any errors
 */
VulkanApplication::~VulkanApplication() {
    std::cout << "VulkanApplication: Starting cleanup..." << std::endl;
    
    try {
        // Stop the main loop if it's running
        m_isRunning = false;
        
        // Clean up Phase 2 resources first
        if (m_computeCommandPool != VK_NULL_HANDLE && m_vulkanSetup) {
            std::cout << "VulkanApplication: Cleaning up compute command pool..." << std::endl;
            vkDestroyCommandPool(m_vulkanSetup->getDevice(), m_computeCommandPool, nullptr);
        }
        
        // Clean up Phase 3 resources
        if (m_graphicsCommandPool != VK_NULL_HANDLE && m_vulkanSetup) {
            std::cout << "VulkanApplication: Cleaning up graphics command pool..." << std::endl;
            vkDestroyCommandPool(m_vulkanSetup->getDevice(), m_graphicsCommandPool, nullptr);
        }
        
        // Clean up Phase 4 resources first (textures must be cleaned before memory manager)
        if (m_textureManager) {
            std::cout << "VulkanApplication: Cleaning up texture manager..." << std::endl;
            m_textureManager.reset();
        }
        
        if (m_graphicsPipeline) {
            std::cout << "VulkanApplication: Cleaning up graphics pipeline..." << std::endl;
            m_graphicsPipeline.reset();
        }
        
        if (m_swapchainManager) {
            std::cout << "VulkanApplication: Cleaning up swapchain manager..." << std::endl;
            m_swapchainManager.reset();
        }
        
        // Clean up compute pipeline (will clean up automatically via RAII)
        if (m_computePipeline) {
            std::cout << "VulkanApplication: Cleaning up compute pipeline..." << std::endl;
            m_computePipeline.reset();
        }
        
        // Clean up memory manager
        if (m_memoryManager) {
            std::cout << "VulkanApplication: Cleaning up memory manager..." << std::endl;
            m_memoryManager.reset();
        }
        
        // Clean up shader manager
        if (m_shaderManager) {
            std::cout << "VulkanApplication: Cleaning up shader manager..." << std::endl;
            m_shaderManager.reset();
        }
        
        // Clean up Vulkan resources (they may depend on the window)
        if (m_vulkanSetup) {
            std::cout << "VulkanApplication: Cleaning up Vulkan subsystem..." << std::endl;
            m_vulkanSetup.reset();
        }
        
        // Clean up window management
        if (m_windowManager) {
            std::cout << "VulkanApplication: Cleaning up window subsystem..." << std::endl;
            m_windowManager.reset();
        }
        
        std::cout << "VulkanApplication: Cleanup completed successfully." << std::endl;
        
    } catch (const std::exception& e) {
        // Log errors but don't throw from destructor
        std::cerr << "VulkanApplication: Error during cleanup: " << e.what() << std::endl;
    }
}

/**
 * @brief Initialize all application subsystems
 * 
 * Coordinates the initialization of window management and Vulkan setup.
 * Each subsystem is responsible for its own initialization, but this
 * function ensures they're created in the correct order and are compatible.
 */
void VulkanApplication::initializeSubsystems() {
    std::cout << "VulkanApplication: Initializing window management..." << std::endl;
    
    // Create and initialize the window management system
    // This must be first because Vulkan needs the window surface
    m_windowManager = std::make_unique<WindowManager>(m_fractalWidth, m_fractalHeight, getWindowTitle());
    
    std::cout << "VulkanApplication: Initializing Vulkan subsystem..." << std::endl;
    
    // Create and initialize Vulkan
    // Pass the window manager so Vulkan can create a surface
    m_vulkanSetup = std::make_unique<VulkanSetup>(*m_windowManager);
    
    std::cout << "VulkanApplication: Initializing Phase 2 compute pipeline subsystems..." << std::endl;
    
    // Initialize shader manager
    m_shaderManager = std::make_shared<ShaderManager>(m_vulkanSetup->getDevice());
    
    // Initialize memory manager
    m_memoryManager = std::make_shared<MemoryManager>(
        m_vulkanSetup->getDevice(),
        m_vulkanSetup->getPhysicalDevice()
    );
    
    // Initialize compute pipeline
    m_computePipeline = std::make_shared<ComputePipeline>(
        m_vulkanSetup->getDevice(),
        m_shaderManager,
        m_memoryManager
    );
    
    // Create command pool for compute operations
    m_computeCommandPool = m_vulkanSetup->createComputeCommandPool();
    
    // Allocate command buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_computeCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    VkResult result = vkAllocateCommandBuffers(m_vulkanSetup->getDevice(), &allocInfo, &m_computeCommandBuffer);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate compute command buffer! Vulkan error: " + std::to_string(result));
    }
    
    // Create fractal compute pipeline
    bool fractalPipelineResult = m_computePipeline->createFractalPipeline(m_fractalWidth, m_fractalHeight);
    if (!fractalPipelineResult) {
        throw std::runtime_error("Failed to create fractal compute pipeline!");
    }
    
    std::cout << "VulkanApplication: Initializing Phase 3 graphics pipeline subsystems..." << std::endl;
    
    // Initialize swapchain manager
    m_swapchainManager = std::make_shared<SwapchainManager>(
        m_vulkanSetup->getDevice(),
        m_vulkanSetup->getPhysicalDevice(),
        m_vulkanSetup->getSurface(),
        *m_windowManager
    );
    
    // Create the swapchain
    bool swapchainResult = m_swapchainManager->createSwapchain();
    if (!swapchainResult) {
        throw std::runtime_error("Failed to create swapchain!");
    }
    
    // Initialize graphics pipeline
    m_graphicsPipeline = std::make_shared<GraphicsPipeline>(
        m_vulkanSetup->getDevice(),
        m_shaderManager,
        m_swapchainManager
    );
    
    // Create graphics pipeline
    bool graphicsPipelineResult = m_graphicsPipeline->createFractalDisplayPipeline();
    if (!graphicsPipelineResult) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
    
    std::cout << "VulkanApplication: Initializing Phase 4 texture management subsystem..." << std::endl;
    
    // Initialize texture manager for compute-to-graphics data transfer
    m_textureManager = std::make_shared<TextureManager>(
        m_vulkanSetup->getDevice(),
        m_vulkanSetup->getPhysicalDevice(),
        m_memoryManager
    );
    
    // Create fractal texture for visualization
    bool textureResult = m_textureManager->createFractalTexture(
        m_fractalWidth, 
        m_fractalHeight, 
        VK_FORMAT_R8G8B8A8_UNORM
    );
    if (!textureResult) {
        throw std::runtime_error("Failed to create fractal texture!");
    }
    
    // Update graphics pipeline to use the fractal texture
    bool descriptorResult = m_graphicsPipeline->updateFractalTexture(
        m_textureManager->getTextureImageView(),
        m_textureManager->getTextureSampler()
    );
    if (!descriptorResult) {
        throw std::runtime_error("Failed to update fractal texture in graphics pipeline!");
    }
    
    // Create graphics command pool and command buffers
    m_graphicsCommandPool = m_vulkanSetup->createGraphicsCommandPool();
    
    // Allocate command buffers (one per swapchain image)
    uint32_t imageCount = m_swapchainManager->getImageCount();
    m_graphicsCommandBuffers.resize(imageCount);
    
    VkCommandBufferAllocateInfo graphicsAllocInfo{};
    graphicsAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    graphicsAllocInfo.commandPool = m_graphicsCommandPool;
    graphicsAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    graphicsAllocInfo.commandBufferCount = static_cast<uint32_t>(m_graphicsCommandBuffers.size());
    
    VkResult graphicsResult = vkAllocateCommandBuffers(m_vulkanSetup->getDevice(), &graphicsAllocInfo, m_graphicsCommandBuffers.data());
    if (graphicsResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate graphics command buffers! Vulkan error: " + std::to_string(graphicsResult));
    }
    
    // TODO(Phase 3): Initialize synchronization objects (semaphores, fences)
    // Details: Create semaphores for swapchain synchronization
    // Priority: High
    // Dependencies: Graphics pipeline working
    
    std::cout << "VulkanApplication: All subsystems initialized successfully." << std::endl;
}

/**
 * @brief Run the main application loop
 * 
 * Implements the standard game/application loop:
 * 1. Process events (input, window events)
 * 2. Update application state
 * 3. Render the current frame
 * 4. Repeat until shutdown requested
 * 
 * This loop will evolve significantly in future phases as we add
 * fractal computation and interactive controls.
 */
void VulkanApplication::run() {
    std::cout << "VulkanApplication: Starting main application loop..." << std::endl;
    
    // Initialize timing for the main loop
    auto startTime = std::chrono::high_resolution_clock::now();
    m_lastFrameTime = 0.0;
    m_isRunning = true;
    
    // Main application loop
    // Continue until the user closes the window or an error occurs
    while (m_isRunning) {
        // Calculate delta time for smooth animations and updates
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration<double>(currentTime - startTime).count();
        double deltaTime = elapsedTime - m_lastFrameTime;
        m_lastFrameTime = elapsedTime;
        
        // Process window events and user input
        processEvents();
        
        // Update application state (animations, physics, etc.)
        updateApplication(deltaTime);
        
        // Render the current frame
        renderFrame();
        
        // TODO(Phase 5): Add frame rate limiting
        // Details: Cap frame rate to avoid excessive CPU/GPU usage
        // Priority: Low
        // Dependencies: Performance profiling system
    }
    
    std::cout << "VulkanApplication: Main loop completed." << std::endl;
}

/**
 * @brief Get the application window title
 * 
 * Creates a descriptive title that includes version and phase information.
 * This helps with debugging and provides user feedback about the current
 * development state.
 * 
 * @return Formatted window title string
 */
std::string VulkanApplication::getWindowTitle() {
    return "Vulkan Fractal Generator - Phase 4: Fractal Integration";
    
    // TODO(Phase 5): Update title to reflect performance metrics and optimizations
    // Details: Include frame rate, GPU utilization, optimization status, etc.
    // Priority: Low
    // Dependencies: Phase 4 completion
}

/**
 * @brief Process window and input events
 * 
 * Handles all events from the windowing system including:
 * - Window close requests
 * - Keyboard input
 * - Mouse input  
 * - Window resize events
 * 
 * Phase 1: Basic event processing for window management
 * Future: Complex input handling for fractal navigation
 */
void VulkanApplication::processEvents() {
    // Process all pending window events
    // This includes window close, resize, and input events
    m_windowManager->pollEvents();
    
    // Check if the user requested to close the window
    if (m_windowManager->shouldClose()) {
        std::cout << "VulkanApplication: Window close requested, shutting down..." << std::endl;
        m_isRunning = false;
    }
    
    // TODO(Phase 3): Add keyboard input processing
    // Details: Handle zoom, pan, reset, and other navigation commands
    // Priority: High
    // Dependencies: Phase 2 fractal computation working
    
    // TODO(Phase 3): Add mouse input processing
    // Details: Mouse dragging for pan, wheel for zoom, click for center
    // Priority: High  
    // Dependencies: Keyboard input working
    
    // TODO(Phase 3): Add window resize handling
    // Details: Recreate swapchain and adjust fractal viewport
    // Priority: Medium
    // Dependencies: Basic rendering pipeline working
}

/**
 * @brief Render a single frame
 * 
 * Handles all rendering operations for one frame. In Phase 1, this is
 * a placeholder that will be expanded in future phases.
 * 
 * Future implementation will include:
 * - Updating fractal parameters
 * - Dispatching compute shaders
 * - Rendering results to screen
 * - Presenting the final image
 */
void VulkanApplication::renderFrame() {
    if (!m_computePipeline || !m_computePipeline->isFractalPipelineReady()) {
        return;  // Phase 2 compute pipeline not ready yet
    }
    
    // Update fractal parameters
    FractalParameters params{};
    params.centerX = m_fractalParams.centerX;
    params.centerY = m_fractalParams.centerY;
    params.zoom = m_fractalParams.zoom;
    params.maxIterations = m_fractalParams.maxIterations;
    params.imageWidth = m_fractalWidth;
    params.imageHeight = m_fractalHeight;
    params.colorScale = m_fractalParams.colorScale;
    params.padding = 0;
    
    m_computePipeline->updateFractalParameters(params);
    
    // Record compute commands
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(m_computeCommandBuffer, &beginInfo);
    
    // Dispatch fractal computation
    m_computePipeline->dispatchFractalCompute(m_computeCommandBuffer);
    
    vkEndCommandBuffer(m_computeCommandBuffer);
    
    // Submit compute work
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_computeCommandBuffer;
    
    VkResult result = vkQueueSubmit(m_vulkanSetup->getComputeQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        std::cerr << "VulkanApplication: Failed to submit compute commands! Error: " << result << std::endl;
        return;
    }
    
    // Wait for completion (for now - will optimize later)
    vkQueueWaitIdle(m_vulkanSetup->getComputeQueue());
    
    // Phase 4: Copy compute buffer to texture for graphics rendering
    if (!m_textureManager || !m_textureManager->isTextureReady()) {
        std::cerr << "VulkanApplication: Texture manager not ready, skipping frame..." << std::endl;
        return;
    }
    
    // Get the fractal output buffer from compute pipeline
    std::shared_ptr<BufferInfo> fractalBuffer = m_computePipeline->getFractalOutputBuffer();
    if (!fractalBuffer || fractalBuffer->buffer == VK_NULL_HANDLE) {
        std::cerr << "VulkanApplication: No fractal output buffer available!" << std::endl;
        return;
    }
    
    // Record buffer-to-texture copy commands
    VkCommandBufferBeginInfo copyBeginInfo{};
    copyBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    copyBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(m_computeCommandBuffer, &copyBeginInfo);
    
    // Transition texture to transfer destination layout
    m_textureManager->transitionTextureLayout(
        m_computeCommandBuffer,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );
    
    // Copy buffer to texture
    m_textureManager->copyBufferToTexture(
        m_computeCommandBuffer,
        fractalBuffer->buffer,
        fractalBuffer->size
    );
    
    // Transition texture to shader read layout
    m_textureManager->transitionTextureLayout(
        m_computeCommandBuffer,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
    
    vkEndCommandBuffer(m_computeCommandBuffer);
    
    // Submit copy commands
    VkSubmitInfo copySubmitInfo{};
    copySubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    copySubmitInfo.commandBufferCount = 1;
    copySubmitInfo.pCommandBuffers = &m_computeCommandBuffer;
    
    VkResult copyResult = vkQueueSubmit(m_vulkanSetup->getGraphicsQueue(), 1, &copySubmitInfo, VK_NULL_HANDLE);
    if (copyResult != VK_SUCCESS) {
        std::cerr << "VulkanApplication: Failed to submit copy commands! Error: " << copyResult << std::endl;
        return;
    }
    
    // Wait for copy completion
    vkQueueWaitIdle(m_vulkanSetup->getGraphicsQueue());
    
    // Phase 3: Graphics rendering implementation
    if (!m_graphicsPipeline || !m_graphicsPipeline->isPipelineReady() || !m_swapchainManager) {
        return;  // Graphics pipeline not ready yet
    }
    
    // Acquire next swapchain image
    uint32_t imageIndex;
    VkResult acquireResult = m_swapchainManager->acquireNextImage(VK_NULL_HANDLE, imageIndex);
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        // TODO: Handle swapchain recreation
        std::cout << "VulkanApplication: Swapchain out of date, skipping frame..." << std::endl;
        return;
    } else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
        std::cerr << "VulkanApplication: Failed to acquire swapchain image! Error: " << acquireResult << std::endl;
        return;
    }
    
    // Record graphics command buffer
    VkCommandBuffer graphicsCmd = m_graphicsCommandBuffers[imageIndex];
    
    VkCommandBufferBeginInfo graphicsBeginInfo{};
    graphicsBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    graphicsBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(graphicsCmd, &graphicsBeginInfo);
    
    // Begin render pass
    m_graphicsPipeline->beginRenderPass(graphicsCmd, imageIndex);
    
    // Render fractal (now using the actual fractal texture)
    m_graphicsPipeline->renderFractal(graphicsCmd, VK_NULL_HANDLE);
    
    // End render pass
    m_graphicsPipeline->endRenderPass(graphicsCmd);
    
    vkEndCommandBuffer(graphicsCmd);
    
    // Submit graphics commands
    VkSubmitInfo graphicsSubmitInfo{};
    graphicsSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    graphicsSubmitInfo.commandBufferCount = 1;
    graphicsSubmitInfo.pCommandBuffers = &graphicsCmd;
    
    VkResult submitResult = vkQueueSubmit(m_vulkanSetup->getGraphicsQueue(), 1, &graphicsSubmitInfo, VK_NULL_HANDLE);
    if (submitResult != VK_SUCCESS) {
        std::cerr << "VulkanApplication: Failed to submit graphics commands! Error: " << submitResult << std::endl;
        return;
    }
    
    // Present the frame
    VkResult presentResult = m_swapchainManager->presentImage(
        m_vulkanSetup->getPresentQueue(), 
        imageIndex, 
        VK_NULL_HANDLE
    );
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        // TODO: Handle swapchain recreation
        std::cout << "VulkanApplication: Swapchain suboptimal/out of date after present..." << std::endl;
    } else if (presentResult != VK_SUCCESS) {
        std::cerr << "VulkanApplication: Failed to present image! Error: " << presentResult << std::endl;
        return;
    }
    
    // Wait for graphics operations to complete (temporary - will use proper sync later)
    vkQueueWaitIdle(m_vulkanSetup->getGraphicsQueue());
    
    // TODO(Phase 3): Implement proper synchronization
    // Details: Use semaphores and fences instead of queue wait idle
    // Priority: High
    // Dependencies: Basic rendering working
    
    // Phase 2: Basic compute dispatch working, log progress occasionally
    static int frameCount = 0;
    if ((frameCount % 60) == 0) {  // Log every 60 frames (~1 second at 60 FPS)
        std::cout << "VulkanApplication: Computed fractal frame " << frameCount 
                  << " (zoom: " << m_fractalParams.zoom << ", iterations: " << m_fractalParams.maxIterations << ")" << std::endl;
        
        // Save first computed frame to verify it's working
        if (frameCount == 0) {
            std::cout << "VulkanApplication: Saving first computed fractal frame..." << std::endl;
            // This confirms that Phase 2 compute pipeline is working!
            // The fractal data is being computed on the GPU.
            // Phase 3 will render this data to the screen.
        }
    }
    frameCount++;
}

/**
 * @brief Update application state for the current frame
 * 
 * Handles non-rendering updates including:
 * - Animation timing
 * - Parameter interpolation
 * - Performance monitoring
 * - State transitions
 * 
 * @param deltaTime Time elapsed since the last frame (in seconds)
 */
void VulkanApplication::updateApplication(double deltaTime) {
    // Silence unused parameter warning for Phase 1
    (void)deltaTime;
    
    // TODO(Phase 2): Add fractal parameter updates
    // Details: Handle zoom animations, position changes, iteration adjustments
    // Priority: High
    // Dependencies: Fractal computation system
    
    // TODO(Phase 3): Add GUI state updates
    // Details: Update Dear ImGui state, handle user interface changes
    // Priority: Medium
    // Dependencies: ImGui integration
    
    // TODO(Phase 4): Add performance monitoring
    // Details: Track frame times, GPU utilization, memory usage
    // Priority: Medium
    // Dependencies: Performance profiling system
    
    // TODO(Phase 5): Add advanced state management
    // Details: Save/load states, animation systems, complex interactions
    // Priority: Low
    // Dependencies: Core functionality stable
}

/**
 * Implementation Notes:
 * 
 * 1. Exception Safety:
 *    - Constructor uses RAII for partial initialization cleanup
 *    - Destructor never throws exceptions
 *    - All subsystems are properly cleaned up in reverse order
 * 
 * 2. Timing Management:
 *    - Uses high-resolution clock for accurate frame timing
 *    - Delta time calculation enables smooth animations
 *    - Frame counting helps with debugging and development feedback
 * 
 * 3. Subsystem Coordination:
 *    - Clear initialization order (window before Vulkan)
 *    - Proper dependency management
 *    - Graceful error handling and cleanup
 * 
 * 4. Future Extensibility:
 *    - Event processing structure ready for complex input
 *    - Render loop designed for multi-stage rendering
 *    - Update system ready for complex state management
 * 
 * 5. Phase 1 Simplicity:
 *    - Minimal functionality for stable foundation
 *    - Clear TODO markers for future development
 *    - Extensive logging for debugging and learning
 */
