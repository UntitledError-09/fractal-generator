/**
 * @file ComputePipeline.cpp
 * @brief Implementation of Vulkan compute pipeline management
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 2
 */

#include "ComputePipeline.h"
#include "MemoryManager.h"
#include "ShaderManager.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

ComputePipeline::ComputePipeline(VkDevice device,
                                 std::shared_ptr<ShaderManager> shaderManager,
                                 std::shared_ptr<MemoryManager> memoryManager)
    : m_device(device), m_shaderManager(shaderManager),
      m_memoryManager(memoryManager), m_fractalPipeline(VK_NULL_HANDLE),
      m_fractalPipelineLayout(VK_NULL_HANDLE),
      m_fractalDescriptorSetLayout(VK_NULL_HANDLE),
      m_descriptorPool(VK_NULL_HANDLE), m_fractalDescriptorSet(VK_NULL_HANDLE),
      m_fractalImageWidth(0), m_fractalImageHeight(0),
      m_fractalPipelineReady(false) {
  std::cout << "[ComputePipeline] Initialized compute pipeline system"
            << std::endl;

  // Create descriptor pool for all pipelines
  m_descriptorPool = createDescriptorPool();
}

ComputePipeline::~ComputePipeline() {
  std::cout << "[ComputePipeline] Cleaning up compute pipeline resources..."
            << std::endl;

  // Clean up fractal pipeline resources
  if (m_fractalPipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(m_device, m_fractalPipeline, nullptr);
  }

  if (m_fractalPipelineLayout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(m_device, m_fractalPipelineLayout, nullptr);
  }

  if (m_fractalDescriptorSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(m_device, m_fractalDescriptorSetLayout,
                                 nullptr);
  }

  if (m_descriptorPool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
  }

  std::cout << "[ComputePipeline] Cleanup complete" << std::endl;
}

bool ComputePipeline::createPipeline(const std::string &pipelineName,
                                     const std::string &shaderName) {
  std::cout << "[ComputePipeline] Creating compute pipeline: " << pipelineName
            << " (shader: " << shaderName << ")" << std::endl;

  // Get the compute shader
  auto shader = m_shaderManager->getShader(shaderName);
  if (!shader) {
    std::cerr << "[ComputePipeline] Shader not found: " << shaderName
              << std::endl;
    return false;
  }

  if (shader->type != ShaderType::COMPUTE) {
    std::cerr << "[ComputePipeline] Shader is not a compute shader: "
              << shaderName << std::endl;
    return false;
  }

  // TODO(Future): Implement generic pipeline creation for multiple compute
  // operations Currently specialized for fractal computation; future expansion
  // for other algorithms
  std::cerr << "[ComputePipeline] Generic pipeline creation not yet implemented"
            << std::endl;
  return false;
}

bool ComputePipeline::createFractalPipeline(uint32_t imageWidth,
                                            uint32_t imageHeight) {
  std::cout << "[ComputePipeline] Creating fractal compute pipeline ("
            << imageWidth << "x" << imageHeight << ")" << std::endl;

  try {
    // Store image dimensions
    m_fractalImageWidth = imageWidth;
    m_fractalImageHeight = imageHeight;

    // Create or load the Mandelbrot compute shader
    auto shader = m_shaderManager->getShader("mandelbrot");
    if (!shader) {
      // Load and compile the shader
      shader = m_shaderManager->loadShaderFromFile(
          "mandelbrot", "shaders/mandelbrot.comp", ShaderType::COMPUTE, "main");
    }

    // Create descriptor set layout
    m_fractalDescriptorSetLayout = createFractalDescriptorSetLayout();

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_fractalDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    VkResult result = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo,
                                             nullptr, &m_fractalPipelineLayout);
    if (result != VK_SUCCESS) {
      throw std::runtime_error(
          "Failed to create pipeline layout! Vulkan error: " +
          std::to_string(result));
    }

    // Create compute pipeline
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = m_fractalPipelineLayout;
    pipelineInfo.stage.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineInfo.stage.module = shader->module;
    pipelineInfo.stage.pName = shader->entryPoint.c_str();

    result =
        vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                 nullptr, &m_fractalPipeline);
    if (result != VK_SUCCESS) {
      throw std::runtime_error(
          "Failed to create compute pipeline! Vulkan error: " +
          std::to_string(result));
    }

    // Create parameter buffer
    size_t paramBufferSize = sizeof(FractalParameters);
    m_fractalParameterBuffer = m_memoryManager->createBuffer(
        "fractal_parameters", paramBufferSize,
        BufferUsage::FRACTAL_PARAMS_BUFFER, MemoryLocation::CPU_TO_GPU,
        true // Persistently mapped for frequent updates
    );

    // Create output buffer
    size_t outputBufferSize =
        imageWidth * imageHeight * sizeof(uint32_t); // RGBA32 format
    m_fractalOutputBuffer = m_memoryManager->createBuffer(
        "fractal_output", outputBufferSize, BufferUsage::FRACTAL_OUTPUT_BUFFER,
        MemoryLocation::GPU_ONLY, false);

    // Allocate and update descriptor set
    m_fractalDescriptorSet = allocateAndUpdateDescriptorSet(
        m_fractalDescriptorSetLayout, m_fractalParameterBuffer,
        m_fractalOutputBuffer);

    m_fractalPipelineReady = true;

    std::cout
        << "[ComputePipeline] Fractal compute pipeline created successfully"
        << std::endl;
    std::cout << "[ComputePipeline] Parameter buffer: "
              << (paramBufferSize / 1024.0f) << " KB" << std::endl;
    std::cout << "[ComputePipeline] Output buffer: "
              << (outputBufferSize / (1024.0f * 1024.0f)) << " MB" << std::endl;

    return true;

  } catch (const std::exception &e) {
    std::cerr << "[ComputePipeline] Failed to create fractal pipeline: "
              << e.what() << std::endl;
    m_fractalPipelineReady = false;
    return false;
  }
}

void ComputePipeline::updateFractalParameters(const FractalParameters &params) {
  if (!m_fractalPipelineReady || !m_fractalParameterBuffer) {
    std::cerr
        << "[ComputePipeline] Fractal pipeline not ready for parameter updates"
        << std::endl;
    return;
  }

  // Copy parameters to the mapped buffer
  if (m_fractalParameterBuffer->mappedData) {
    std::memcpy(m_fractalParameterBuffer->mappedData, &params,
                sizeof(FractalParameters));
  } else {
    std::cerr << "[ComputePipeline] Parameter buffer not mapped!" << std::endl;
  }
}

void ComputePipeline::dispatchFractalCompute(VkCommandBuffer commandBuffer,
                                             uint32_t workGroupSizeX,
                                             uint32_t workGroupSizeY) {
  if (!m_fractalPipelineReady) {
    std::cerr << "[ComputePipeline] Fractal pipeline not ready for dispatch"
              << std::endl;
    return;
  }

  // Calculate dispatch info
  ComputeDispatchInfo dispatchInfo =
      calculateDispatchInfo(m_fractalImageWidth, m_fractalImageHeight,
                            workGroupSizeX, workGroupSizeY);

  // Bind compute pipeline
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    m_fractalPipeline);

  // Bind descriptor set
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          m_fractalPipelineLayout, 0, 1,
                          &m_fractalDescriptorSet, 0, nullptr);

  // Dispatch compute work
  vkCmdDispatch(commandBuffer, dispatchInfo.groupCountX,
                dispatchInfo.groupCountY, dispatchInfo.groupCountZ);

  // std::cout << "[ComputePipeline] Dispatched fractal compute: "
  //           << dispatchInfo.groupCountX << "x" << dispatchInfo.groupCountY
  //           << " work groups (" << workGroupSizeX << "x" << workGroupSizeY <<
  //           " local size)" << std::endl;
}

std::shared_ptr<BufferInfo> ComputePipeline::getFractalOutputBuffer() const {
  return m_fractalOutputBuffer;
}

bool ComputePipeline::getFractalData(std::vector<uint32_t> &outputData) {
  if (!m_fractalPipelineReady || !m_fractalOutputBuffer) {
    return false;
  }

  size_t pixelCount = m_fractalImageWidth * m_fractalImageHeight;
  outputData.resize(pixelCount);

  try {
    // Note: This requires the output buffer to be host-visible for direct
    // download In a real implementation, we might need a staging buffer for
    // GPU-only memory
    m_memoryManager->downloadBufferData(m_fractalOutputBuffer,
                                        outputData.data(),
                                        pixelCount * sizeof(uint32_t));
    return true;

  } catch (const std::exception &e) {
    std::cerr << "[ComputePipeline] Failed to download fractal data: "
              << e.what() << std::endl;
    return false;
  }
}

bool ComputePipeline::isFractalPipelineReady() const {
  return m_fractalPipelineReady;
}

void ComputePipeline::getFractalDimensions(uint32_t &width,
                                           uint32_t &height) const {
  width = m_fractalImageWidth;
  height = m_fractalImageHeight;
}

VkDescriptorSetLayout ComputePipeline::createFractalDescriptorSetLayout() {
  // Descriptor bindings for fractal computation
  VkDescriptorSetLayoutBinding bindings[2] = {};

  // Binding 0: Uniform buffer for fractal parameters
  bindings[0].binding = 0;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindings[0].descriptorCount = 1;
  bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  bindings[0].pImmutableSamplers = nullptr;

  // Binding 1: Storage buffer for output data
  bindings[1].binding = 1;
  bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  bindings[1].descriptorCount = 1;
  bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
  bindings[1].pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 2;
  layoutInfo.pBindings = bindings;

  VkDescriptorSetLayout descriptorSetLayout;
  VkResult result = vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr,
                                                &descriptorSetLayout);

  if (result != VK_SUCCESS) {
    throw std::runtime_error(
        "Failed to create descriptor set layout! Vulkan error: " +
        std::to_string(result));
  }

  return descriptorSetLayout;
}

VkDescriptorPool ComputePipeline::createDescriptorPool(uint32_t maxSets) {
  VkDescriptorPoolSize poolSizes[2] = {};

  // Uniform buffers
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = maxSets;

  // Storage buffers
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[1].descriptorCount = maxSets;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 2;
  poolInfo.pPoolSizes = poolSizes;
  poolInfo.maxSets = maxSets;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

  VkDescriptorPool descriptorPool;
  VkResult result =
      vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &descriptorPool);

  if (result != VK_SUCCESS) {
    throw std::runtime_error(
        "Failed to create descriptor pool! Vulkan error: " +
        std::to_string(result));
  }

  return descriptorPool;
}

VkDescriptorSet ComputePipeline::allocateAndUpdateDescriptorSet(
    VkDescriptorSetLayout layout, std::shared_ptr<BufferInfo> parameterBuffer,
    std::shared_ptr<BufferInfo> outputBuffer) {
  // Allocate descriptor set
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = m_descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &layout;

  VkDescriptorSet descriptorSet;
  VkResult result =
      vkAllocateDescriptorSets(m_device, &allocInfo, &descriptorSet);
  if (result != VK_SUCCESS) {
    throw std::runtime_error(
        "Failed to allocate descriptor set! Vulkan error: " +
        std::to_string(result));
  }

  // Update descriptor set
  VkWriteDescriptorSet descriptorWrites[2] = {};

  // Parameter buffer descriptor
  VkDescriptorBufferInfo paramBufferInfo{};
  paramBufferInfo.buffer = parameterBuffer->buffer;
  paramBufferInfo.offset = 0;
  paramBufferInfo.range = parameterBuffer->size;

  descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[0].dstSet = descriptorSet;
  descriptorWrites[0].dstBinding = 0;
  descriptorWrites[0].dstArrayElement = 0;
  descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrites[0].descriptorCount = 1;
  descriptorWrites[0].pBufferInfo = &paramBufferInfo;

  // Output buffer descriptor
  VkDescriptorBufferInfo outputBufferInfo{};
  outputBufferInfo.buffer = outputBuffer->buffer;
  outputBufferInfo.offset = 0;
  outputBufferInfo.range = outputBuffer->size;

  descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrites[1].dstSet = descriptorSet;
  descriptorWrites[1].dstBinding = 1;
  descriptorWrites[1].dstArrayElement = 0;
  descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrites[1].descriptorCount = 1;
  descriptorWrites[1].pBufferInfo = &outputBufferInfo;

  vkUpdateDescriptorSets(m_device, 2, descriptorWrites, 0, nullptr);

  return descriptorSet;
}

ComputeDispatchInfo ComputePipeline::calculateDispatchInfo(
    uint32_t imageWidth, uint32_t imageHeight, uint32_t workGroupSizeX,
    uint32_t workGroupSizeY) {
  ComputeDispatchInfo info;

  // Calculate number of work groups needed to cover the entire image
  // Round up to ensure all pixels are covered
  info.groupCountX = (imageWidth + workGroupSizeX - 1) / workGroupSizeX;
  info.groupCountY = (imageHeight + workGroupSizeY - 1) / workGroupSizeY;
  info.groupCountZ = 1; // 2D fractal computation

  return info;
}
