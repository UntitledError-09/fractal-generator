/**
 * @file GraphicsPipeline.cpp
 * @brief Implementation of graphics pipeline for fractal visualization
 *
 * This file implements the graphics pipeline system for rendering
 * computed fractal data to the screen using a fullscreen quad.
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 3
 */

#include "GraphicsPipeline.h"
#include "ShaderManager.h"
#include "SwapchainManager.h"

#include <iostream>
#include <vector>

/**
 * @brief Constructor
 */
GraphicsPipeline::GraphicsPipeline(
    VkDevice device, std::shared_ptr<ShaderManager> shaderManager,
    std::shared_ptr<SwapchainManager> swapchainManager)
    : m_device(device), m_shaderManager(shaderManager),
      m_swapchainManager(swapchainManager), m_renderPass(VK_NULL_HANDLE),
      m_pipelineLayout(VK_NULL_HANDLE), m_graphicsPipeline(VK_NULL_HANDLE),
      m_descriptorSetLayout(VK_NULL_HANDLE), m_descriptorPool(VK_NULL_HANDLE),
      m_descriptorSet(VK_NULL_HANDLE), m_vertexShader(VK_NULL_HANDLE),
      m_fragmentShader(VK_NULL_HANDLE), m_pipelineReady(false) {
  std::cout << "GraphicsPipeline: Initializing graphics pipeline..."
            << std::endl;
}

/**
 * @brief Destructor
 */
GraphicsPipeline::~GraphicsPipeline() {
  std::cout << "GraphicsPipeline: Cleaning up graphics pipeline..."
            << std::endl;

  // Clean up framebuffers
  cleanupFramebuffers();

  // Clean up shader modules
  if (m_vertexShader != VK_NULL_HANDLE) {
    vkDestroyShaderModule(m_device, m_vertexShader, nullptr);
  }
  if (m_fragmentShader != VK_NULL_HANDLE) {
    vkDestroyShaderModule(m_device, m_fragmentShader, nullptr);
  }

  // Clean up pipeline objects
  if (m_graphicsPipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
  }
  if (m_pipelineLayout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
  }
  if (m_descriptorPool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
  }
  if (m_descriptorSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
  }
  if (m_renderPass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
  }

  std::cout << "GraphicsPipeline: Cleanup completed." << std::endl;
}

/**
 * @brief Create the graphics pipeline for fractal display
 */
bool GraphicsPipeline::createFractalDisplayPipeline() {
  std::cout << "GraphicsPipeline: Creating fractal display pipeline..."
            << std::endl;

  try {
    // Create descriptor set layout first
    if (!createDescriptorSetLayout()) {
      std::cerr << "GraphicsPipeline: Failed to create descriptor set layout!"
                << std::endl;
      return false;
    }

    // Create descriptor pool
    if (!createDescriptorPool()) {
      std::cerr << "GraphicsPipeline: Failed to create descriptor pool!"
                << std::endl;
      return false;
    }

    // Create descriptor set
    if (!createDescriptorSet()) {
      std::cerr << "GraphicsPipeline: Failed to create descriptor set!"
                << std::endl;
      return false;
    }

    // Create render pass
    if (!createRenderPass()) {
      std::cerr << "GraphicsPipeline: Failed to create render pass!"
                << std::endl;
      return false;
    }

    // Create the graphics pipeline
    if (!createPipeline()) {
      std::cerr << "GraphicsPipeline: Failed to create graphics pipeline!"
                << std::endl;
      return false;
    }

    // Create framebuffers
    if (!createFramebuffers()) {
      std::cerr << "GraphicsPipeline: Failed to create framebuffers!"
                << std::endl;
      return false;
    }

    m_pipelineReady = true;
    std::cout
        << "GraphicsPipeline: Fractal display pipeline created successfully."
        << std::endl;
    return true;

  } catch (const std::exception &e) {
    std::cerr << "GraphicsPipeline: Exception during pipeline creation: "
              << e.what() << std::endl;
    return false;
  }
}

/**
 * @brief Check if the graphics pipeline is ready
 */
bool GraphicsPipeline::isPipelineReady() const { return m_pipelineReady; }

/**
 * @brief Begin a render pass
 */
void GraphicsPipeline::beginRenderPass(VkCommandBuffer commandBuffer,
                                       uint32_t imageIndex) {
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = m_renderPass;
  renderPassInfo.framebuffer = m_framebuffers[imageIndex];

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = m_swapchainManager->getExtent();

  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);
}

/**
 * @brief Render the fractal fullscreen quad
 */
void GraphicsPipeline::renderFractal(VkCommandBuffer commandBuffer,
                                     VkImageView fractalTexture) {
  // Bind the graphics pipeline
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_graphicsPipeline);

  // Set viewport and scissor (dynamic state)
  VkExtent2D extent = m_swapchainManager->getExtent();

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(extent.width);
  viewport.height = static_cast<float>(extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = extent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  // Bind descriptor set with fractal texture
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);

  // Draw fullscreen quad (4 vertices as triangle strip)
  vkCmdDraw(commandBuffer, 4, 1, 0, 0);
}

/**
 * @brief Update the fractal texture binding
 */
bool GraphicsPipeline::updateFractalTexture(VkImageView textureImageView,
                                            VkSampler textureSampler) {
  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = textureImageView;
  imageInfo.sampler = textureSampler;

  VkWriteDescriptorSet descriptorWrite{};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = m_descriptorSet;
  descriptorWrite.dstBinding = 0;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pImageInfo = &imageInfo;

  vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
  return true;
}

/**
 * @brief End the render pass
 */
void GraphicsPipeline::endRenderPass(VkCommandBuffer commandBuffer) {
  vkCmdEndRenderPass(commandBuffer);
}

/**
 * @brief Recreate pipeline for swapchain changes
 */
void GraphicsPipeline::recreateForSwapchain() {
  std::cout << "GraphicsPipeline: Recreating pipeline for swapchain changes..."
            << std::endl;

  // Clean up old framebuffers
  cleanupFramebuffers();

  // Recreate framebuffers with new swapchain images
  if (!createFramebuffers()) {
    std::cerr << "GraphicsPipeline: Failed to recreate framebuffers!"
              << std::endl;
    m_pipelineReady = false;
  }
}

/**
 * @brief Create descriptor set layout
 */
bool GraphicsPipeline::createDescriptorSetLayout() {
  // Create descriptor set layout for fractal texture sampling
  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 0;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType =
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &samplerLayoutBinding;

  VkResult result = vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr,
                                                &m_descriptorSetLayout);
  if (result != VK_SUCCESS) {
    std::cerr
        << "GraphicsPipeline: Failed to create descriptor set layout! Error: "
        << result << std::endl;
    return false;
  }

  return true;
}

/**
 * @brief Create render pass
 */
bool GraphicsPipeline::createRenderPass() {
  // Color attachment (swapchain image)
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = m_swapchainManager->getFormat();
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  VkResult result =
      vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass);
  if (result != VK_SUCCESS) {
    std::cerr << "GraphicsPipeline: Failed to create render pass! Error: "
              << result << std::endl;
    return false;
  }

  return true;
}

/**
 * @brief Create the graphics pipeline
 */
bool GraphicsPipeline::createPipeline() {
  // Load and compile shaders
  std::cout << "GraphicsPipeline: Loading vertex shader..." << std::endl;
  auto vertexShaderInfo = m_shaderManager->loadShaderFromFile(
      "fullscreen_vertex", "shaders/fullscreen.vert", ShaderType::VERTEX);
  if (!vertexShaderInfo) {
    std::cerr << "GraphicsPipeline: Failed to load vertex shader!" << std::endl;
    return false;
  }
  m_vertexShader = vertexShaderInfo->module;

  std::cout << "GraphicsPipeline: Loading fragment shader..." << std::endl;
  auto fragmentShaderInfo = m_shaderManager->loadShaderFromFile(
      "fractal_display_fragment", "shaders/fractal_display.frag",
      ShaderType::FRAGMENT);
  if (!fragmentShaderInfo) {
    std::cerr << "GraphicsPipeline: Failed to load fragment shader!"
              << std::endl;
    return false;
  }
  m_fragmentShader = fragmentShaderInfo->module;

  // Shader stage creation
  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = m_vertexShader;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = m_fragmentShader;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  // Vertex input (none - fullscreen triangle)
  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = 0;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;

  // Input assembly
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // Dynamic viewport and scissor
  std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                               VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

  // Viewport state (will be set dynamically)
  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  // Rasterization
  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  // Color blending
  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  // Pipeline layout
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

  VkResult result = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo,
                                           nullptr, &m_pipelineLayout);
  if (result != VK_SUCCESS) {
    std::cerr << "GraphicsPipeline: Failed to create pipeline layout! Error: "
              << result << std::endl;
    return false;
  }

  // Create graphics pipeline
  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = m_pipelineLayout;
  pipelineInfo.renderPass = m_renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                     nullptr, &m_graphicsPipeline);
  if (result != VK_SUCCESS) {
    std::cerr << "GraphicsPipeline: Failed to create graphics pipeline! Error: "
              << result << std::endl;
    return false;
  }

  return true;
}

/**
 * @brief Create framebuffers
 */
bool GraphicsPipeline::createFramebuffers() {
  auto swapchainImages = m_swapchainManager->getImageViews();
  m_framebuffers.resize(swapchainImages.size());

  for (size_t i = 0; i < swapchainImages.size(); i++) {
    VkImageView attachments[] = {swapchainImages[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = m_swapchainManager->getExtent().width;
    framebufferInfo.height = m_swapchainManager->getExtent().height;
    framebufferInfo.layers = 1;

    VkResult result = vkCreateFramebuffer(m_device, &framebufferInfo, nullptr,
                                          &m_framebuffers[i]);
    if (result != VK_SUCCESS) {
      std::cerr << "GraphicsPipeline: Failed to create framebuffer " << i
                << "! Error: " << result << std::endl;
      return false;
    }
  }

  return true;
}

/**
 * @brief Cleanup framebuffers
 */
void GraphicsPipeline::cleanupFramebuffers() {
  for (auto framebuffer : m_framebuffers) {
    if (framebuffer != VK_NULL_HANDLE) {
      vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
  }
  m_framebuffers.clear();
}

/**
 * @brief Create descriptor pool
 */
bool GraphicsPipeline::createDescriptorPool() {
  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSize.descriptorCount = 1;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;
  poolInfo.maxSets = 1;

  VkResult result =
      vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool);
  if (result != VK_SUCCESS) {
    std::cerr << "GraphicsPipeline: Failed to create descriptor pool! Error: "
              << result << std::endl;
    return false;
  }

  return true;
}

/**
 * @brief Create descriptor set
 */
bool GraphicsPipeline::createDescriptorSet() {
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = m_descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &m_descriptorSetLayout;

  VkResult result =
      vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSet);
  if (result != VK_SUCCESS) {
    std::cerr << "GraphicsPipeline: Failed to allocate descriptor set! Error: "
              << result << std::endl;
    return false;
  }

  return true;
}
