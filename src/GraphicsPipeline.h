/**
 * @file GraphicsPipeline.h
 * @brief Graphics pipeline management for fractal visualization
 * 
 * This class manages the graphics pipeline used to render the computed
 * fractal data to the screen. It handles vertex/fragment shaders,
 * render passes, and drawing a fullscreen quad.
 * 
 * Phase 3 Focus:
 * - Graphics pipeline creation
 * - Render pass management
 * - Fullscreen quad rendering
 * - Texture display from compute buffer
 * 
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 3
 */

#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

// Forward declarations
class ShaderManager;
class SwapchainManager;

/**
 * @class GraphicsPipeline
 * @brief Manages the graphics rendering pipeline for fractal display
 * 
 * This class encapsulates all graphics pipeline operations needed to
 * display the computed fractal data on screen using a fullscreen quad.
 */
class GraphicsPipeline {
public:
    /**
     * @brief Constructor
     * 
     * @param device Vulkan logical device
     * @param shaderManager Shared shader manager for compilation
     * @param swapchainManager Shared swapchain manager for format info
     */
    GraphicsPipeline(
        VkDevice device,
        std::shared_ptr<ShaderManager> shaderManager,
        std::shared_ptr<SwapchainManager> swapchainManager
    );
    
    /**
     * @brief Destructor - cleanup all resources
     */
    ~GraphicsPipeline();
    
    // Non-copyable
    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
    
    /**
     * @brief Create the graphics pipeline for fractal display
     * 
     * Creates all necessary resources:
     * - Vertex and fragment shaders
     * - Render pass
     * - Graphics pipeline
     * - Framebuffers
     * 
     * @return true if successful, false otherwise
     */
    bool createFractalDisplayPipeline();
    
    /**
     * @brief Check if the graphics pipeline is ready for rendering
     * 
     * @return true if pipeline is ready, false otherwise
     */
    bool isPipelineReady() const;
    
    /**
     * @brief Begin a render pass for the current frame
     * 
     * @param commandBuffer Command buffer to record commands
     * @param imageIndex Current swapchain image index
     */
    void beginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    
    /**
     * @brief Render the fractal fullscreen quad
     * 
     * @param commandBuffer Command buffer to record commands
     * @param fractalTexture The fractal texture to display
     */
    void renderFractal(VkCommandBuffer commandBuffer, VkImageView fractalTexture);
    
    /**
     * @brief Update the fractal texture binding
     * 
     * Updates the descriptor set to bind a new fractal texture.
     * 
     * @param textureImageView Image view of the fractal texture
     * @param textureSampler Sampler for the fractal texture
     * @return true if successful, false otherwise
     */
    bool updateFractalTexture(VkImageView textureImageView, VkSampler textureSampler);
    
    /**
     * @brief End the current render pass
     * 
     * @param commandBuffer Command buffer to record commands
     */
    void endRenderPass(VkCommandBuffer commandBuffer);
    
    /**
     * @brief Recreate pipeline resources for swapchain changes
     * 
     * Called when the swapchain is recreated due to window resize.
     */
    void recreateForSwapchain();

private:
    /**
     * @brief Create the render pass
     * 
     * @return true if successful, false otherwise
     */
    bool createRenderPass();
    
    /**
     * @brief Create the graphics pipeline
     * 
     * @return true if successful, false otherwise
     */
    bool createPipeline();
    
    /**
     * @brief Create framebuffers for each swapchain image
     * 
     * @return true if successful, false otherwise
     */
    bool createFramebuffers();
    
    /**
     * @brief Create descriptor set layout for texture sampling
     * 
     * @return true if successful, false otherwise
     */
    bool createDescriptorSetLayout();
    
    /**
     * @brief Create descriptor pool for descriptor set allocation
     * 
     * @return true if successful, false otherwise
     */
    bool createDescriptorPool();
    
    /**
     * @brief Create descriptor set for texture binding
     * 
     * @return true if successful, false otherwise
     */
    bool createDescriptorSet();
    
    /**
     * @brief Cleanup framebuffers
     */
    void cleanupFramebuffers();
    
    // Vulkan objects
    VkDevice m_device;
    std::shared_ptr<ShaderManager> m_shaderManager;
    std::shared_ptr<SwapchainManager> m_swapchainManager;
    
    // Graphics pipeline objects
    VkRenderPass m_renderPass;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSet m_descriptorSet;
    
    // Framebuffers (one per swapchain image)
    std::vector<VkFramebuffer> m_framebuffers;
    
    // Shader modules
    VkShaderModule m_vertexShader;
    VkShaderModule m_fragmentShader;
    
    // Pipeline state
    bool m_pipelineReady;
};
