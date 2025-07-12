/**
 * @file GuiManager.cpp
 * @brief Implementation of GUI management for interactive fractal controls
 *
 * This file implements the Dear ImGui integration for real-time fractal
 * parameter control and visualization. It provides a modern, extensible
 * GUI framework designed for future VSCode-style layout expansion.
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 5 - Interactive GUI Implementation
 */

#include "GuiManager.h"
#include "GraphicsPipeline.h"
#include "SwapchainManager.h"
#include "VulkanSetup.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <algorithm>
#include <cmath>
#include <iostream>

/**
 * @brief Constructor
 */
GuiManager::GuiManager(std::shared_ptr<VulkanSetup> vulkanSetup,
                       std::shared_ptr<SwapchainManager> swapchainManager,
                       std::shared_ptr<GraphicsPipeline> graphicsPipeline,
                       GLFWwindow *window)
    : m_vulkanSetup(vulkanSetup), m_swapchainManager(swapchainManager),
      m_graphicsPipeline(graphicsPipeline), m_window(window),
      m_imguiDescriptorPool(VK_NULL_HANDLE), m_initialized(false),
      m_windowWidth(800), m_windowHeight(600) {
  std::cout << "GuiManager: Initializing ImGui integration..." << std::endl;

  // Get initial window size
  int width, height;
  glfwGetWindowSize(m_window, &width, &height);
  m_windowWidth = static_cast<uint32_t>(width);
  m_windowHeight = static_cast<uint32_t>(height);
}

/**
 * @brief Destructor
 */
GuiManager::~GuiManager() {
  std::cout << "GuiManager: Cleaning up ImGui resources..." << std::endl;

  if (m_initialized) {
    // Wait for device to be idle before cleanup
    if (m_vulkanSetup && m_vulkanSetup->getDevice() != VK_NULL_HANDLE) {
      vkDeviceWaitIdle(m_vulkanSetup->getDevice());
    }

    // Cleanup ImGui
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup Vulkan resources
    if (m_imguiDescriptorPool != VK_NULL_HANDLE) {
      vkDestroyDescriptorPool(m_vulkanSetup->getDevice(), m_imguiDescriptorPool,
                              nullptr);
    }
  }
}

/**
 * @brief Initialize ImGui with Vulkan backend
 */
bool GuiManager::initialize() {
  std::cout << "GuiManager: Setting up ImGui context..." << std::endl;

  // Create ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard navigation
  // Note: Docking disabled for now, will enable in future version with docking
  // branch io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable
  // docking (future VSCode-style layout)

  // Setup ImGui style
  setupImGuiStyle();

  // Create descriptor pool for ImGui
  if (!createImGuiDescriptorPool()) {
    std::cerr << "GuiManager: Failed to create ImGui descriptor pool!"
              << std::endl;
    return false;
  }

  // Initialize GLFW backend
  if (!ImGui_ImplGlfw_InitForVulkan(m_window, true)) {
    std::cerr << "GuiManager: Failed to initialize ImGui GLFW backend!"
              << std::endl;
    return false;
  }

  // Initialize Vulkan backend
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = m_vulkanSetup->getInstance();
  init_info.PhysicalDevice = m_vulkanSetup->getPhysicalDevice();
  init_info.Device = m_vulkanSetup->getDevice();
  init_info.QueueFamily = m_vulkanSetup->getGraphicsQueueFamily();
  init_info.Queue = m_vulkanSetup->getGraphicsQueue();
  init_info.DescriptorPool = m_imguiDescriptorPool;
  init_info.RenderPass = m_graphicsPipeline->getRenderPass();
  init_info.MinImageCount = m_swapchainManager->getImageCount();
  init_info.ImageCount = m_swapchainManager->getImageCount();
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

  if (!ImGui_ImplVulkan_Init(&init_info)) {
    std::cerr << "GuiManager: Failed to initialize ImGui Vulkan backend!"
              << std::endl;
    return false;
  }

  m_initialized = true;
  std::cout << "GuiManager: ImGui initialization completed successfully."
            << std::endl;
  return true;
}

/**
 * @brief Begin new ImGui frame
 */
void GuiManager::beginFrame() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

/**
 * @brief Render fractal parameter controls
 */
bool GuiManager::renderControls(FractalUIParameters &parameters) {
  bool changed = false;

  // Render main menu bar
  renderMenuBar();

  // Simple two-panel layout for now (future: full docking system)
  // Left panel for controls, right area for fractal display

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                  ImGuiWindowFlags_NoResize |
                                  ImGuiWindowFlags_NoMove;

  // Control panel (left side)
  ImGui::SetNextWindowPos(ImVec2(0, 20)); // Account for menu bar
  ImGui::SetNextWindowSize(ImVec2(m_controlPanelWidth, m_windowHeight - 20));

  ImGui::Begin("FractalControls", nullptr, window_flags);
  changed = renderFractalControls(parameters);
  ImGui::End();

  // Performance metrics panel (if enabled)
  if (m_showMetrics) {
    renderMetricsPanel();
  }

  // Demo window (if enabled)
  if (m_showDemoWindow) {
    ImGui::ShowDemoWindow(&m_showDemoWindow);
  }

  return changed;
}

/**
 * @brief End ImGui frame and record render commands
 */
void GuiManager::endFrame(VkCommandBuffer commandBuffer) {
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

/**
 * @brief Check if GUI wants to capture mouse input
 */
bool GuiManager::wantCaptureMouse() const {
  return ImGui::GetIO().WantCaptureMouse;
}

/**
 * @brief Check if GUI wants to capture keyboard input
 */
bool GuiManager::wantCaptureKeyboard() const {
  return ImGui::GetIO().WantCaptureKeyboard;
}

/**
 * @brief Handle window resize events
 */
void GuiManager::handleResize(uint32_t width, uint32_t height) {
  m_windowWidth = width;
  m_windowHeight = height;
}

/**
 * @brief Get fractal viewport dimensions
 */
void GuiManager::getFractalViewport(uint32_t &width, uint32_t &height) const {
  // Calculate available space for fractal rendering after accounting for GUI
  // panels
  width = m_windowWidth - static_cast<uint32_t>(m_controlPanelWidth);
  height = m_windowHeight - 20; // Account for menu bar

  // Ensure minimum size
  width = std::max(width, 100u);
  height = std::max(height, 100u);
}

/**
 * @brief Create ImGui descriptor pool
 */
bool GuiManager::createImGuiDescriptorPool() {
  VkDescriptorPoolSize pool_sizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
  pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
  pool_info.pPoolSizes = pool_sizes;

  VkResult result = vkCreateDescriptorPool(
      m_vulkanSetup->getDevice(), &pool_info, nullptr, &m_imguiDescriptorPool);
  return result == VK_SUCCESS;
}

/**
 * @brief Render main menu bar
 */
void GuiManager::renderMenuBar() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("New", "Ctrl+N");
      ImGui::MenuItem("Open", "Ctrl+O");
      ImGui::MenuItem("Save", "Ctrl+S");
      ImGui::Separator();
      ImGui::MenuItem("Export Image", "Ctrl+E");
      ImGui::Separator();
      if (ImGui::MenuItem("Exit", "Alt+F4")) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
      ImGui::MenuItem("Control Panel", nullptr, nullptr,
                      false); // Always visible for now
      ImGui::MenuItem("Performance Metrics", nullptr, &m_showMetrics);
      ImGui::Separator();
      ImGui::MenuItem("ImGui Demo", nullptr, &m_showDemoWindow);
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
      ImGui::MenuItem("About");
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}

/**
 * @brief Render fractal control panel
 */
bool GuiManager::renderFractalControls(FractalUIParameters &parameters) {
  bool changed = false;

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));

  // Collapsible sections for better organization
  if (ImGui::CollapsingHeader("Fractal Explorer",
                              ImGuiTreeNodeFlags_DefaultOpen)) {

    // Fractal type selection
    const char *fractalTypes[] = {"Mandelbrot", "Julia Set", "Burning Ship"};
    if (ImGui::Combo("Fractal Type", &parameters.fractalType, fractalTypes,
                     IM_ARRAYSIZE(fractalTypes))) {
      changed = true;
    }
  }

  if (ImGui::CollapsingHeader("Navigation", ImGuiTreeNodeFlags_DefaultOpen)) {

    // Center position controls with Blender-style input/slider combo
    // Movement scale based on zoom level for precision control
    float moveScale = 2.0f / std::max(1.0f, parameters.zoom * 0.1f);
    float moveMin = parameters.centerX - moveScale;
    float moveMax = parameters.centerX + moveScale;

    ImGui::Text("Center Position");

    // Center X with both slider and input
    ImGui::PushItemWidth(120);
    if (ImGui::DragFloat("##CenterX", &parameters.centerX, moveScale * 0.001f,
                         moveMin, moveMax, "%.8f")) {
      changed = true;
    }
    ImGui::SameLine();
    ImGui::Text("Center X");

    // Center Y with both slider and input
    ImGui::PushItemWidth(120);
    if (ImGui::DragFloat("##CenterY", &parameters.centerY, moveScale * 0.001f,
                         parameters.centerY - moveScale,
                         parameters.centerY + moveScale, "%.8f")) {
      changed = true;
    }
    ImGui::SameLine();
    ImGui::Text("Center Y");
    ImGui::PopItemWidth();

    // Zoom control (logarithmic scale for better UX)
    ImGui::Text("Zoom Level");
    float logZoom = std::log10(std::max(1.0f, parameters.zoom));
    ImGui::PushItemWidth(150);
    if (ImGui::SliderFloat("##Zoom", &logZoom, 0.0f, 8.0f, "10^%.2f")) {
      parameters.zoom = std::pow(10.0f, logZoom);
      changed = true;
    }
    ImGui::SameLine();
    ImGui::PushItemWidth(80);
    float tempZoom = parameters.zoom;
    if (ImGui::InputFloat("##ZoomInput", &tempZoom, 0.0f, 0.0f, "%.2e")) {
      if (tempZoom > 0) {
        parameters.zoom = tempZoom;
        changed = true;
      }
    }
    ImGui::PopItemWidth();
    ImGui::PopItemWidth();
  }

  if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {

    // Resolution controls
    ImGui::Text("Resolution");
    ImGui::PushItemWidth(80);
    bool resChanged = false;
    if (ImGui::InputInt("##Width", &parameters.resolutionWidth, 0, 0)) {
      parameters.resolutionWidth =
          std::max(100, std::min(4096, parameters.resolutionWidth));
      resChanged = true;
    }
    ImGui::SameLine();
    ImGui::Text("x");
    ImGui::SameLine();
    if (ImGui::InputInt("##Height", &parameters.resolutionHeight, 0, 0)) {
      parameters.resolutionHeight =
          std::max(100, std::min(4096, parameters.resolutionHeight));
      resChanged = true;
    }
    ImGui::PopItemWidth();

    if (resChanged) {
      changed = true;
    }

    // Common resolution presets
    if (ImGui::Button("800x600")) {
      parameters.resolutionWidth = 800;
      parameters.resolutionHeight = 600;
      changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("1920x1080")) {
      parameters.resolutionWidth = 1920;
      parameters.resolutionHeight = 1080;
      changed = true;
    }
    if (ImGui::Button("1024x1024")) {
      parameters.resolutionWidth = 1024;
      parameters.resolutionHeight = 1024;
      changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("2048x2048")) {
      parameters.resolutionWidth = 2048;
      parameters.resolutionHeight = 2048;
      changed = true;
    }

    ImGui::Separator();

    // Iteration count with slider and input
    ImGui::Text("Quality");
    ImGui::PushItemWidth(120);
    if (ImGui::SliderInt("##MaxIter", &parameters.maxIterations, 10, 2000)) {
      changed = true;
    }
    ImGui::SameLine();
    ImGui::PushItemWidth(60);
    if (ImGui::InputInt("##MaxIterInput", &parameters.maxIterations, 0, 0)) {
      parameters.maxIterations =
          std::max(10, std::min(5000, parameters.maxIterations));
      changed = true;
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::Text("Max Iterations");
    ImGui::PopItemWidth();

    // Color scale with slider and input
    ImGui::PushItemWidth(120);
    if (ImGui::SliderFloat("##ColorScale", &parameters.colorScale, 0.1f, 10.0f,
                           "%.2f")) {
      changed = true;
    }
    ImGui::SameLine();
    ImGui::PushItemWidth(60);
    if (ImGui::InputFloat("##ColorScaleInput", &parameters.colorScale, 0.0f,
                          0.0f, "%.3f")) {
      parameters.colorScale =
          std::max(0.01f, std::min(50.0f, parameters.colorScale));
      changed = true;
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::Text("Color Scale");
    ImGui::PopItemWidth();
  }

  ImGui::Separator();

  // Action buttons
  if (ImGui::Button("Reset View")) {
    parameters.centerX = -0.5f;
    parameters.centerY = 0.0f;
    parameters.zoom = 1.0f;
    changed = true;
  }

  ImGui::SameLine();
  if (ImGui::Button("High Quality")) {
    parameters.maxIterations = 1000;
    changed = true;
  }

  // Status information
  if (ImGui::CollapsingHeader("Status")) {
    ImGui::Text("Center: (%.8f, %.8f)", parameters.centerX, parameters.centerY);
    ImGui::Text("Zoom: %.2e", parameters.zoom);
    ImGui::Text("Iterations: %d", parameters.maxIterations);
    ImGui::Text("Resolution: %dx%d", parameters.resolutionWidth,
                parameters.resolutionHeight);
    float aspectRatio =
        (float)parameters.resolutionWidth / parameters.resolutionHeight;
    ImGui::Text("Aspect Ratio: %.3f", aspectRatio);
  }

  ImGui::PopStyleVar();

  if (changed) {
    parameters.parametersChanged = true;
    parameters.needsRecompute = true;
  }

  return changed;
}

/**
 * @brief Render performance metrics panel
 */
void GuiManager::renderMetricsPanel() {
  ImGui::Begin("Performance Metrics", &m_showMetrics);

  ImGui::Text("Frame Rate: %.1f FPS", ImGui::GetIO().Framerate);
  ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

  ImGui::Separator();
  ImGui::Text("Window: %ux%u", m_windowWidth, m_windowHeight);

  uint32_t fractalWidth, fractalHeight;
  getFractalViewport(fractalWidth, fractalHeight);
  ImGui::Text("Fractal Viewport: %ux%u", fractalWidth, fractalHeight);

  ImGui::End();
}

/**
 * @brief Apply ImGui styling
 */
void GuiManager::setupImGuiStyle() {
  ImGuiStyle &style = ImGui::GetStyle();

  // Modern dark theme with VSCode-inspired colors
  style.WindowRounding = 6.0f;
  style.FrameRounding = 3.0f;
  style.ScrollbarRounding = 3.0f;
  style.GrabRounding = 3.0f;
  style.TabRounding = 3.0f;

  style.WindowPadding = ImVec2(8.0f, 8.0f);
  style.FramePadding = ImVec2(8.0f, 4.0f);
  style.ItemSpacing = ImVec2(8.0f, 6.0f);

  // Color scheme - dark theme with modern accents
  ImVec4 *colors = style.Colors;
  colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
  colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.26f, 1.00f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.24f, 0.25f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.32f, 0.32f, 0.33f, 1.00f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.26f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.36f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.46f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.56f, 1.00f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.00f, 0.47f, 0.84f, 0.40f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.56f, 1.00f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.00f, 0.47f, 0.84f, 0.31f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.47f, 0.84f, 0.80f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);
  colors[ImGuiCol_Tab] = ImVec4(0.06f, 0.06f, 0.07f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.47f, 0.84f, 0.80f);
  colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);
}
