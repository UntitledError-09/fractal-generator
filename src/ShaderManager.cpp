/**
 * @file ShaderManager.cpp
 * @brief Implementation of Vulkan shader compilation and management
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 2
 */

#include "ShaderManager.h"
#include <fstream>
#include <iostream>
#include <shaderc/shaderc.hpp>
#include <stdexcept>
#include <sys/stat.h>

ShaderManager::ShaderManager(VkDevice device) : m_device(device) {
  std::cout << "[ShaderManager] Initialized shader manager" << std::endl;
}

ShaderManager::~ShaderManager() {
  std::cout << "[ShaderManager] Cleaning up " << m_shaders.size() << " shaders"
            << std::endl;
  clearShaders();
}

std::shared_ptr<ShaderInfo>
ShaderManager::compileShader(const std::string &name, const std::string &source,
                             ShaderType type, const std::string &entryPoint) {
  std::cout << "[ShaderManager] Compiling shader: " << name << std::endl;

  // Check if shader is already cached
  auto existing = getShader(name);
  if (existing) {
    std::cout << "[ShaderManager] Using cached shader: " << name << std::endl;
    return existing;
  }

  try {
    // Compile GLSL to SPIR-V
    std::vector<uint32_t> spirvCode =
        compileGLSLToSPIRV(source, type, entryPoint, name);

    // Create Vulkan shader module
    VkShaderModule module = createVulkanShaderModule(spirvCode);

    // Create shader info
    auto shaderInfo = std::make_shared<ShaderInfo>();
    shaderInfo->module = module;
    shaderInfo->type = type;
    shaderInfo->entryPoint = entryPoint;
    shaderInfo->spirvCode = std::move(spirvCode);
    shaderInfo->sourceCode = source;

    // Cache the shader
    m_shaders[name] = shaderInfo;

    std::cout << "[ShaderManager] Successfully compiled shader: " << name
              << " (SPIR-V size: " << shaderInfo->spirvCode.size() * 4
              << " bytes)" << std::endl;

    return shaderInfo;

  } catch (const std::exception &e) {
    std::cerr << "[ShaderManager] Failed to compile shader '" << name
              << "': " << e.what() << std::endl;
    throw;
  }
}

std::shared_ptr<ShaderInfo>
ShaderManager::loadShaderFromFile(const std::string &name,
                                  const std::string &filePath, ShaderType type,
                                  const std::string &entryPoint) {
  std::cout << "[ShaderManager] Loading shader from file: " << filePath
            << std::endl;

  try {
    // Read shader source from file
    std::string source = readFile(filePath);

    // Compile the shader
    return compileShader(name, source, type, entryPoint);

  } catch (const std::exception &e) {
    std::cerr << "[ShaderManager] Failed to load shader from file '" << filePath
              << "': " << e.what() << std::endl;
    throw;
  }
}

std::shared_ptr<ShaderInfo> ShaderManager::createShaderModule(
    const std::string &name, const std::vector<uint32_t> &spirvCode,
    ShaderType type, const std::string &entryPoint) {
  std::cout << "[ShaderManager] Creating shader module: " << name << std::endl;

  // Check if shader is already cached
  auto existing = getShader(name);
  if (existing) {
    std::cout << "[ShaderManager] Using cached shader: " << name << std::endl;
    return existing;
  }

  try {
    // Create Vulkan shader module
    VkShaderModule module = createVulkanShaderModule(spirvCode);

    // Create shader info
    auto shaderInfo = std::make_shared<ShaderInfo>();
    shaderInfo->module = module;
    shaderInfo->type = type;
    shaderInfo->entryPoint = entryPoint;
    shaderInfo->spirvCode = spirvCode;
    shaderInfo->sourceCode = ""; // No source available for pre-compiled SPIR-V

    // Cache the shader
    m_shaders[name] = shaderInfo;

    std::cout << "[ShaderManager] Successfully created shader module: " << name
              << std::endl;

    return shaderInfo;

  } catch (const std::exception &e) {
    std::cerr << "[ShaderManager] Failed to create shader module '" << name
              << "': " << e.what() << std::endl;
    throw;
  }
}

std::shared_ptr<ShaderInfo>
ShaderManager::getShader(const std::string &name) const {
  auto it = m_shaders.find(name);
  if (it != m_shaders.end()) {
    return it->second;
  }
  return nullptr;
}

bool ShaderManager::removeShader(const std::string &name) {
  auto it = m_shaders.find(name);
  if (it != m_shaders.end()) {
    std::cout << "[ShaderManager] Removing shader: " << name << std::endl;

    // Destroy the Vulkan shader module
    vkDestroyShaderModule(m_device, it->second->module, nullptr);

    // Remove from cache
    m_shaders.erase(it);
    return true;
  }
  return false;
}

void ShaderManager::clearShaders() {
  for (const auto &[name, shader] : m_shaders) {
    std::cout << "[ShaderManager] Destroying shader module: " << name
              << std::endl;
    vkDestroyShaderModule(m_device, shader->module, nullptr);
  }
  m_shaders.clear();
}

std::vector<std::string> ShaderManager::getShaderNames() const {
  std::vector<std::string> names;
  names.reserve(m_shaders.size());

  for (const auto &[name, shader] : m_shaders) {
    names.push_back(name);
  }

  return names;
}

std::vector<uint32_t>
ShaderManager::compileGLSLToSPIRV(const std::string &source, ShaderType type,
                                  const std::string &entryPoint,
                                  const std::string &fileName) {
  // Initialize shaderc compiler
  shaderc::Compiler compiler;
  shaderc::CompileOptions options;

  // Set compilation options
  options.SetOptimizationLevel(shaderc_optimization_level_performance);
  options.SetWarningsAsErrors();
  options.SetGenerateDebugInfo();

  // Convert shader type to shaderc kind
  shaderc_shader_kind shadercKind;
  switch (type) {
  case ShaderType::VERTEX:
    shadercKind = shaderc_vertex_shader;
    break;
  case ShaderType::FRAGMENT:
    shadercKind = shaderc_fragment_shader;
    break;
  case ShaderType::COMPUTE:
    shadercKind = shaderc_compute_shader;
    break;
  case ShaderType::GEOMETRY:
    shadercKind = shaderc_geometry_shader;
    break;
  case ShaderType::TESSELLATION_CONTROL:
    shadercKind = shaderc_tess_control_shader;
    break;
  case ShaderType::TESSELLATION_EVALUATION:
    shadercKind = shaderc_tess_evaluation_shader;
    break;
  default:
    throw std::runtime_error("Unsupported shader type");
  }

  // Compile the shader
  std::cout << "[ShaderManager] Compiling " << fileName
            << " (entry: " << entryPoint << ")" << std::endl;

  shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
      source, shadercKind, fileName.c_str(), entryPoint.c_str(), options);

  // Check compilation status
  if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
    std::string error = "Shader compilation failed:\n";
    error += result.GetErrorMessage();
    throw std::runtime_error(error);
  }

  // Extract SPIR-V bytecode
  std::vector<uint32_t> spirvCode(result.cbegin(), result.cend());

  std::cout << "[ShaderManager] Compilation successful. SPIR-V size: "
            << spirvCode.size() * 4 << " bytes" << std::endl;

  return spirvCode;
}

VkShaderModule ShaderManager::createVulkanShaderModule(
    const std::vector<uint32_t> &spirvCode) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
  createInfo.pCode = spirvCode.data();

  VkShaderModule shaderModule;
  VkResult result =
      vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module! Vulkan error: " +
                             std::to_string(result));
  }

  return shaderModule;
}

uint32_t ShaderManager::shaderTypeToShadercKind(ShaderType type) {
  switch (type) {
  case ShaderType::VERTEX:
    return shaderc_vertex_shader;
  case ShaderType::FRAGMENT:
    return shaderc_fragment_shader;
  case ShaderType::COMPUTE:
    return shaderc_compute_shader;
  case ShaderType::GEOMETRY:
    return shaderc_geometry_shader;
  case ShaderType::TESSELLATION_CONTROL:
    return shaderc_tess_control_shader;
  case ShaderType::TESSELLATION_EVALUATION:
    return shaderc_tess_evaluation_shader;
  default:
    throw std::runtime_error("Unsupported shader type");
  }
}

VkShaderStageFlagBits ShaderManager::shaderTypeToVulkanStage(ShaderType type) {
  switch (type) {
  case ShaderType::VERTEX:
    return VK_SHADER_STAGE_VERTEX_BIT;
  case ShaderType::FRAGMENT:
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  case ShaderType::COMPUTE:
    return VK_SHADER_STAGE_COMPUTE_BIT;
  case ShaderType::GEOMETRY:
    return VK_SHADER_STAGE_GEOMETRY_BIT;
  case ShaderType::TESSELLATION_CONTROL:
    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
  case ShaderType::TESSELLATION_EVALUATION:
    return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
  default:
    throw std::runtime_error("Unsupported shader type");
  }
}

std::string ShaderManager::readFile(const std::string &filePath) {
  std::ifstream file(filePath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filePath);
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::string content;
  content.resize(fileSize);

  file.seekg(0);
  file.read(content.data(), fileSize);
  file.close();

  return content;
}

bool ShaderManager::enableHotReload(const std::string &name,
                                    const std::string &filePath) {
  try {
    // Check if shader exists
    auto shader = getShader(name);
    if (!shader) {
      std::cerr << "[ShaderManager] Cannot enable hot-reload: shader '" << name
                << "' not found" << std::endl;
      return false;
    }

    // Get file modification time
    std::time_t modTime = getFileModTime(filePath);

    // Store hot-reload info
    HotReloadInfo hotReloadInfo;
    hotReloadInfo.filePath = filePath;
    hotReloadInfo.type = shader->type;
    hotReloadInfo.entryPoint = shader->entryPoint;
    hotReloadInfo.lastModTime = modTime;

    m_hotReloadShaders[name] = hotReloadInfo;

    std::cout << "[ShaderManager] Hot-reload enabled for shader: " << name
              << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "[ShaderManager] Failed to enable hot-reload for '" << name
              << "': " << e.what() << std::endl;
    return false;
  }
}

void ShaderManager::disableHotReload(const std::string &name) {
  auto it = m_hotReloadShaders.find(name);
  if (it != m_hotReloadShaders.end()) {
    m_hotReloadShaders.erase(it);
    std::cout << "[ShaderManager] Hot-reload disabled for shader: " << name
              << std::endl;
  }
}

std::vector<std::string> ShaderManager::checkForUpdates() {
  std::vector<std::string> recompiledShaders;

  for (auto &[name, hotReloadInfo] : m_hotReloadShaders) {
    try {
      std::time_t currentModTime = getFileModTime(hotReloadInfo.filePath);

      // Check if file was modified
      if (currentModTime > hotReloadInfo.lastModTime) {
        std::cout << "[ShaderManager] Detected change in shader file: "
                  << hotReloadInfo.filePath << std::endl;

        // Read updated source
        std::string updatedSource = readFile(hotReloadInfo.filePath);

        // Remove old shader from cache
        removeShader(name);

        // Recompile with updated source
        auto recompiledShader = compileShader(
            name, updatedSource, hotReloadInfo.type, hotReloadInfo.entryPoint);

        if (recompiledShader) {
          // Update modification time
          hotReloadInfo.lastModTime = currentModTime;
          recompiledShaders.push_back(name);
          std::cout << "[ShaderManager] Successfully recompiled shader: "
                    << name << std::endl;
        } else {
          std::cerr << "[ShaderManager] Failed to recompile shader: " << name
                    << std::endl;
        }
      }
    } catch (const std::exception &e) {
      std::cerr << "[ShaderManager] Error checking shader '" << name
                << "' for updates: " << e.what() << std::endl;
    }
  }

  return recompiledShaders;
}

std::time_t ShaderManager::getFileModTime(const std::string &filePath) {
  std::ifstream file(filePath, std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot access file: " + filePath);
  }

  // Get file stats for modification time
  struct stat fileStat;
  if (stat(filePath.c_str(), &fileStat) != 0) {
    throw std::runtime_error("Cannot get file stats: " + filePath);
  }

  return fileStat.st_mtime;
}
