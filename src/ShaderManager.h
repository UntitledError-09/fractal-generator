/**
 * @file ShaderManager.h
 * @brief Vulkan shader compilation and management
 *
 * This class handles shader compilation from GLSL to SPIR-V and manages
 * shader module creation. It provides utilities for loading shaders at
 * runtime and compile-time validation.
 *
 * Phase 2 Focus:
 * - GLSL to SPIR-V compilation
 * - Shader module creation and management
 * - Compute shader support for fractal generation
 * - Runtime shader error handling
 *
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 2
 */

#pragma once

#include <ctime>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

/**
 * @enum ShaderType
 * @brief Types of shaders supported by the shader manager
 */
enum class ShaderType {
  VERTEX,
  FRAGMENT,
  COMPUTE,
  GEOMETRY,
  TESSELLATION_CONTROL,
  TESSELLATION_EVALUATION
};

/**
 * @struct ShaderInfo
 * @brief Information about a compiled shader
 */
struct ShaderInfo {
  VkShaderModule module;           ///< Vulkan shader module handle
  ShaderType type;                 ///< Type of shader
  std::string entryPoint;          ///< Entry point function name
  std::vector<uint32_t> spirvCode; ///< Compiled SPIR-V bytecode
  std::string sourceCode;          ///< Original GLSL source (for debugging)
};

/**
 * @class ShaderManager
 * @brief Manages shader compilation and Vulkan shader modules
 *
 * This class provides a high-level interface for working with shaders in
 * Vulkan. It handles the compilation process from GLSL to SPIR-V and manages
 * shader module lifecycle.
 *
 * Key Features:
 * - GLSL to SPIR-V compilation using glslang or shaderc
 * - Shader module creation and destruction
 * - Caching compiled shaders for performance
 * - Error handling and diagnostic messages
 * - Support for compute shaders (primary focus for Phase 2)
 *
 * Design Philosophy:
 * - Hide shader compilation complexity
 * - Provide clear error messages for shader issues
 * - Cache compiled shaders to avoid recompilation
 * - Prepared for hot-reloading in future phases
 */
class ShaderManager {
public:
  /**
   * @brief Constructor - Initialize shader manager
   *
   * @param device Vulkan logical device for shader module creation
   */
  explicit ShaderManager(VkDevice device);

  /**
   * @brief Destructor - Clean up all shader modules
   */
  ~ShaderManager();

  // Disable copy and move for simplicity
  ShaderManager(const ShaderManager &) = delete;
  ShaderManager &operator=(const ShaderManager &) = delete;
  ShaderManager(ShaderManager &&) = delete;
  ShaderManager &operator=(ShaderManager &&) = delete;

  /**
   * @brief Compile GLSL source to SPIR-V and create shader module
   *
   * @param name Unique name for the shader (for caching and debugging)
   * @param source GLSL source code
   * @param type Type of shader to compile
   * @param entryPoint Entry point function name (default: "main")
   * @return Pointer to ShaderInfo containing compiled shader data
   *
   * @throws std::runtime_error If compilation or module creation fails
   */
  std::shared_ptr<ShaderInfo>
  compileShader(const std::string &name, const std::string &source,
                ShaderType type, const std::string &entryPoint = "main");

  /**
   * @brief Load and compile shader from file
   *
   * @param name Unique name for the shader
   * @param filePath Path to GLSL shader file
   * @param type Type of shader to compile
   * @param entryPoint Entry point function name (default: "main")
   * @return Pointer to ShaderInfo containing compiled shader data
   *
   * @throws std::runtime_error If file reading, compilation, or module creation
   * fails
   */
  std::shared_ptr<ShaderInfo>
  loadShaderFromFile(const std::string &name, const std::string &filePath,
                     ShaderType type, const std::string &entryPoint = "main");

  /**
   * @brief Create shader module from pre-compiled SPIR-V bytecode
   *
   * @param name Unique name for the shader
   * @param spirvCode Compiled SPIR-V bytecode
   * @param type Type of shader
   * @param entryPoint Entry point function name (default: "main")
   * @return Pointer to ShaderInfo containing shader data
   *
   * @throws std::runtime_error If module creation fails
   */
  std::shared_ptr<ShaderInfo>
  createShaderModule(const std::string &name,
                     const std::vector<uint32_t> &spirvCode, ShaderType type,
                     const std::string &entryPoint = "main");

  /**
   * @brief Get cached shader by name
   *
   * @param name Name of the shader to retrieve
   * @return Pointer to ShaderInfo if found, nullptr otherwise
   */
  std::shared_ptr<ShaderInfo> getShader(const std::string &name) const;

  /**
   * @brief Remove shader from cache and destroy module
   *
   * @param name Name of the shader to remove
   * @return true if shader was found and removed, false otherwise
   */
  bool removeShader(const std::string &name);

  /**
   * @brief Clear all cached shaders and destroy modules
   */
  void clearShaders();

  /**
   * @brief Get list of all cached shader names
   *
   * @return Vector of shader names
   */
  std::vector<std::string> getShaderNames() const;

  /**
   * @brief Enable hot-reloading for a shader file
   *
   * Monitors the shader file for changes and automatically recompiles
   * when modifications are detected. Useful for development.
   *
   * @param name Shader name to enable hot-reloading for
   * @param filePath Path to the shader file to monitor
   * @return True if hot-reloading was enabled successfully
   */
  bool enableHotReload(const std::string &name, const std::string &filePath);

  /**
   * @brief Disable hot-reloading for a shader
   *
   * @param name Shader name to disable hot-reloading for
   */
  void disableHotReload(const std::string &name);

  /**
   * @brief Check for shader file changes and recompile if needed
   *
   * Should be called periodically (e.g., once per frame) to check
   * for file modifications and trigger recompilation.
   *
   * @return Vector of shader names that were recompiled
   */
  std::vector<std::string> checkForUpdates();

  // TODO(Phase 4): Add shader optimization levels
  // TODO(Phase 5): Add shader reflection for automatic descriptor set layout

private:
  /**
   * @brief Compile GLSL source to SPIR-V bytecode
   *
   * Uses the shaderc library to compile GLSL source code to SPIR-V.
   * Provides detailed error messages if compilation fails.
   *
   * @param source GLSL source code
   * @param type Type of shader to compile
   * @param entryPoint Entry point function name
   * @param fileName Optional filename for error reporting
   * @return Compiled SPIR-V bytecode
   *
   * @throws std::runtime_error If compilation fails
   */
  std::vector<uint32_t>
  compileGLSLToSPIRV(const std::string &source, ShaderType type,
                     const std::string &entryPoint,
                     const std::string &fileName = "shader");

  /**
   * @brief Create Vulkan shader module from SPIR-V bytecode
   *
   * @param spirvCode Compiled SPIR-V bytecode
   * @return VkShaderModule handle
   *
   * @throws std::runtime_error If module creation fails
   */
  VkShaderModule
  createVulkanShaderModule(const std::vector<uint32_t> &spirvCode);

  /**
   * @brief Convert ShaderType enum to shaderc shader kind
   *
   * @param type ShaderType to convert
   * @return Corresponding shaderc_shader_kind
   */
  uint32_t shaderTypeToShadercKind(ShaderType type);

  /**
   * @brief Convert ShaderType enum to VkShaderStageFlagBits
   *
   * @param type ShaderType to convert
   * @return Corresponding VkShaderStageFlagBits
   */
  VkShaderStageFlagBits shaderTypeToVulkanStage(ShaderType type);

  /**
   * @brief Read entire file into string
   *
   * @param filePath Path to file to read
   * @return File contents as string
   *
   * @throws std::runtime_error If file cannot be read
   */
  std::string readFile(const std::string &filePath);

  /**
   * @brief Get file modification time
   *
   * @param filePath Path to file
   * @return Last modification time
   */
  std::time_t getFileModTime(const std::string &filePath);

  // Member variables
  VkDevice m_device; ///< Vulkan logical device
  std::unordered_map<std::string, std::shared_ptr<ShaderInfo>>
      m_shaders; ///< Cached shaders

  // Hot-reload support
  struct HotReloadInfo {
    std::string filePath;
    ShaderType type;
    std::string entryPoint;
    std::time_t lastModTime;
  };
  std::unordered_map<std::string, HotReloadInfo>
      m_hotReloadShaders; ///< Hot-reload tracking
};

/**
 * Implementation Notes:
 *
 * 1. Shader Compilation:
 *    - Uses shaderc library for GLSL to SPIR-V compilation
 *    - Comprehensive error reporting with line numbers
 *    - Support for multiple shader types (focus on compute for Phase 2)
 *    - Caching to avoid recompilation overhead
 *
 * 2. Memory Management:
 *    - RAII pattern for shader module lifecycle
 *    - Shared pointers for safe shader sharing between pipelines
 *    - Automatic cleanup in destructor
 *
 * 3. Error Handling:
 *    - Clear error messages for compilation failures
 *    - File I/O error handling
 *    - Vulkan API error checking
 *
 * 4. Future Extensions:
 *    - Hot-reloading for development workflow
 *    - Shader optimization levels
 *    - Automatic descriptor set layout generation from reflection
 *    - Include system for modular shaders
 */
