/**
 * @file ComputePipeline.h
 * @brief Vulkan compute pipeline management for fractal generation
 * 
 * This class manages compute pipelines, descriptor sets, and command buffers
 * for GPU-accelerated fractal computation. It provides a high-level interface
 * for dispatching compute work and managing compute resources.
 * 
 * Phase 2 Focus:
 * - Compute pipeline creation and management
 * - Descriptor set layout and allocation
 * - Command buffer recording for compute dispatch
 * - Basic Mandelbrot set computation
 * 
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 2
 */

#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <string>

// Forward declarations
class ShaderManager;
class MemoryManager;
struct ShaderInfo;
struct BufferInfo;

/**
 * @struct ComputeDispatchInfo
 * @brief Information for a compute dispatch operation
 */
struct ComputeDispatchInfo {
    uint32_t groupCountX;    ///< Number of work groups in X dimension
    uint32_t groupCountY;    ///< Number of work groups in Y dimension
    uint32_t groupCountZ;    ///< Number of work groups in Z dimension
};

/**
 * @struct FractalParameters
 * @brief Parameters for fractal computation
 * 
 * This structure contains all the parameters needed to compute fractals.
 * It will be uploaded to a uniform buffer for use by compute shaders.
 */
struct FractalParameters {
    float centerX;           ///< Center X coordinate in fractal space
    float centerY;           ///< Center Y coordinate in fractal space
    float zoom;              ///< Zoom level (higher = more zoomed in)
    uint32_t maxIterations;  ///< Maximum iterations for fractal computation
    uint32_t imageWidth;     ///< Output image width in pixels
    uint32_t imageHeight;    ///< Output image height in pixels
    float colorScale;        ///< Scale factor for color mapping
    uint32_t padding;        ///< Padding for alignment
};

/**
 * @class ComputePipeline
 * @brief High-level compute pipeline management for fractal generation
 * 
 * This class provides a complete compute pipeline system specifically designed
 * for fractal generation. It manages shaders, descriptor sets, buffers, and
 * command recording.
 * 
 * Key Features:
 * - Automatic compute pipeline creation from shaders
 * - Descriptor set layout generation and management
 * - Buffer binding and parameter updates
 * - Command buffer recording and dispatch
 * - Resource lifecycle management
 * 
 * Design Philosophy:
 * - Hide Vulkan compute complexity behind simple APIs
 * - Optimize for fractal computation workloads
 * - Provide both convenience and fine-grained control
 * - Prepared for multiple fractal types and algorithms
 */
class ComputePipeline {
public:
    /**
     * @brief Constructor - Initialize compute pipeline system
     * 
     * @param device Vulkan logical device
     * @param shaderManager Shader manager for loading compute shaders
     * @param memoryManager Memory manager for buffer allocation
     */
    ComputePipeline(
        VkDevice device,
        std::shared_ptr<ShaderManager> shaderManager,
        std::shared_ptr<MemoryManager> memoryManager
    );
    
    /**
     * @brief Destructor - Clean up all compute resources
     */
    ~ComputePipeline();
    
    // Disable copy and move for simplicity
    ComputePipeline(const ComputePipeline&) = delete;
    ComputePipeline& operator=(const ComputePipeline&) = delete;
    ComputePipeline(ComputePipeline&&) = delete;
    ComputePipeline& operator=(ComputePipeline&&) = delete;
    
    /**
     * @brief Create compute pipeline from shader
     * 
     * @param pipelineName Unique name for the pipeline
     * @param shaderName Name of the compute shader to use
     * @return true if pipeline created successfully, false otherwise
     * 
     * @throws std::runtime_error If pipeline creation fails
     */
    bool createPipeline(const std::string& pipelineName, const std::string& shaderName);
    
    /**
     * @brief Create fractal computation pipeline
     * 
     * Creates a specialized pipeline for Mandelbrot set computation with
     * appropriate descriptor set layout and parameter buffers.
     * 
     * @param imageWidth Width of the output fractal image
     * @param imageHeight Height of the output fractal image
     * @return true if pipeline created successfully, false otherwise
     */
    bool createFractalPipeline(uint32_t imageWidth, uint32_t imageHeight);
    
    /**
     * @brief Update fractal parameters
     * 
     * Updates the uniform buffer with new fractal computation parameters.
     * 
     * @param params New fractal parameters
     */
    void updateFractalParameters(const FractalParameters& params);
    
    /**
     * @brief Dispatch fractal computation
     * 
     * Records and submits commands to compute a fractal using the current parameters.
     * 
     * @param commandBuffer Command buffer to record into
     * @param workGroupSizeX Local work group size in X dimension (default: 16)
     * @param workGroupSizeY Local work group size in Y dimension (default: 16)
     */
    void dispatchFractalCompute(
        VkCommandBuffer commandBuffer,
        uint32_t workGroupSizeX = 16,
        uint32_t workGroupSizeY = 16
    );
    
    /**
     * @brief Get the fractal output buffer
     * 
     * Returns the buffer containing computed fractal data. The buffer contains
     * pixel values that can be used for rendering or saving to file.
     * 
     * @return Pointer to fractal output buffer, nullptr if not created
     */
    std::shared_ptr<BufferInfo> getFractalOutputBuffer() const;
    
    /**
     * @brief Get computed fractal data
     * 
     * Downloads the computed fractal data from GPU memory to host memory.
     * The data is returned as an array of 32-bit RGBA values.
     * 
     * @param outputData Vector to store the fractal data
     * @return true if data retrieved successfully, false otherwise
     */
    bool getFractalData(std::vector<uint32_t>& outputData);
    
    /**
     * @brief Check if fractal pipeline is ready
     * 
     * @return true if fractal pipeline is created and ready for use
     */
    bool isFractalPipelineReady() const;
    
    /**
     * @brief Get current fractal image dimensions
     * 
     * @param width Output parameter for image width
     * @param height Output parameter for image height
     */
    void getFractalDimensions(uint32_t& width, uint32_t& height) const;
    
    // TODO(Phase 3): Add multiple fractal type support
    // TODO(Phase 3): Add real-time parameter animation
    // TODO(Phase 4): Add multi-pipeline support for complex fractals
    // TODO(Phase 5): Add GPU profiling and optimization

private:
    /**
     * @brief Create descriptor set layout for fractal computation
     * 
     * Creates a descriptor set layout that includes:
     * - Uniform buffer for fractal parameters
     * - Storage buffer for output data
     * 
     * @return VkDescriptorSetLayout handle
     */
    VkDescriptorSetLayout createFractalDescriptorSetLayout();
    
    /**
     * @brief Create descriptor pool for allocating descriptor sets
     * 
     * @param maxSets Maximum number of descriptor sets to allocate
     * @return VkDescriptorPool handle
     */
    VkDescriptorPool createDescriptorPool(uint32_t maxSets = 100);
    
    /**
     * @brief Allocate and update descriptor set
     * 
     * @param layout Descriptor set layout to use
     * @param parameterBuffer Buffer containing fractal parameters
     * @param outputBuffer Buffer for fractal output data
     * @return VkDescriptorSet handle
     */
    VkDescriptorSet allocateAndUpdateDescriptorSet(
        VkDescriptorSetLayout layout,
        std::shared_ptr<BufferInfo> parameterBuffer,
        std::shared_ptr<BufferInfo> outputBuffer
    );
    
    /**
     * @brief Calculate optimal work group count for given dimensions
     * 
     * @param imageWidth Image width in pixels
     * @param imageHeight Image height in pixels
     * @param workGroupSizeX Local work group size X
     * @param workGroupSizeY Local work group size Y
     * @return ComputeDispatchInfo with calculated group counts
     */
    ComputeDispatchInfo calculateDispatchInfo(
        uint32_t imageWidth,
        uint32_t imageHeight,
        uint32_t workGroupSizeX,
        uint32_t workGroupSizeY
    );
    
    // Member variables
    VkDevice m_device;                                          ///< Vulkan logical device
    std::shared_ptr<ShaderManager> m_shaderManager;            ///< Shader management
    std::shared_ptr<MemoryManager> m_memoryManager;            ///< Memory management
    
    // Pipeline resources
    VkPipeline m_fractalPipeline;                              ///< Fractal compute pipeline
    VkPipelineLayout m_fractalPipelineLayout;                  ///< Fractal pipeline layout
    VkDescriptorSetLayout m_fractalDescriptorSetLayout;        ///< Fractal descriptor layout
    VkDescriptorPool m_descriptorPool;                         ///< Descriptor pool
    VkDescriptorSet m_fractalDescriptorSet;                    ///< Fractal descriptor set
    
    // Fractal-specific resources
    std::shared_ptr<BufferInfo> m_fractalParameterBuffer;      ///< Fractal parameters buffer
    std::shared_ptr<BufferInfo> m_fractalOutputBuffer;         ///< Fractal output buffer
    uint32_t m_fractalImageWidth;                              ///< Current fractal image width
    uint32_t m_fractalImageHeight;                             ///< Current fractal image height
    bool m_fractalPipelineReady;                               ///< Whether fractal pipeline is ready
};

/**
 * Implementation Notes:
 * 
 * 1. Compute Pipeline Design:
 *    - Specialized for fractal computation workloads
 *    - Descriptor sets optimized for parameter updates
 *    - Work group sizes tuned for GPU architecture
 *    - Resource management integrated with memory manager
 * 
 * 2. Fractal Computation:
 *    - Uses 32-bit RGBA output format for compatibility
 *    - Parameters stored in uniform buffer for efficiency
 *    - Output buffer sized for full image resolution
 *    - Supports real-time parameter updates
 * 
 * 3. Performance Considerations:
 *    - Work group size optimization for GPU utilization
 *    - Memory layout optimized for compute access patterns
 *    - Minimal CPU-GPU synchronization points
 *    - Prepared for asynchronous computation
 * 
 * 4. Resource Management:
 *    - RAII pattern for automatic cleanup
 *    - Integration with existing memory manager
 *    - Proper Vulkan object lifecycle management
 * 
 * 5. Future Extensions:
 *    - Multiple fractal algorithms (Julia, Newton, etc.)
 *    - Multi-precision arithmetic for deep zooms
 *    - GPU profiling and optimization features
 *    - Real-time parameter animation system
 */
