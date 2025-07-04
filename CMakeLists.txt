# Vulkan Hybrid CPU-GPU Fractal Generator
# Phase 1: Foundation Setup
# 
# This CMake file sets up the build system for a cross-platform Vulkan application
# with GLFW windowing and future Dear ImGui integration.

cmake_minimum_required(VERSION 3.20)
project(VulkanFractalGenerator VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard requirement
# C++20 provides concepts, ranges, and other modern features we'll use
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable debug information and warnings
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# TODO(Phase 1): Add conditional compilation for different platforms
# Details: Need to handle macOS Metal interop, Windows D3D12 compatibility
# Priority: Medium
# Dependencies: Basic Vulkan setup working

# Find required packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Find shaderc for shader compilation
find_library(SHADERC_LIBRARY shaderc_shared PATHS /opt/homebrew/lib)
if(NOT SHADERC_LIBRARY)
    message(FATAL_ERROR "shaderc library not found. Install with: brew install shaderc")
endif()

# TODO(Phase 3): Add Dear ImGui when we implement GUI controls
# Details: Will need to integrate ImGui with Vulkan backend
# Priority: High
# Dependencies: Phase 2 compute pipeline completion

# Create the main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/VulkanApplication.cpp
    src/VulkanSetup.cpp
    src/WindowManager.cpp
    src/ShaderManager.cpp
    src/MemoryManager.cpp
    src/ComputePipeline.cpp
    src/SwapchainManager.cpp
    src/GraphicsPipeline.cpp
    src/TextureManager.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Vulkan_INCLUDE_DIRS}
    /opt/homebrew/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Vulkan_LIBRARIES}
    glfw
    ${SHADERC_LIBRARY}
)

# Platform-specific configurations
if(APPLE)
    # macOS requires special handling for Vulkan via MoltenVK
    # MoltenVK translates Vulkan calls to Metal
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_MACOS_MVK)
    
    # TODO(Phase 1): Verify MoltenVK is properly installed on macOS
    # Details: Check for MoltenVK library and provide installation instructions
    # Priority: High
    # Dependencies: None
    
elseif(WIN32)
    # Windows native Vulkan support
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)
    
    # TODO(Phase 5): Add Windows-specific optimizations
    # Details: Windows has different memory management patterns
    # Priority: Low
    # Dependencies: Core functionality working
    
elseif(UNIX)
    # Linux with X11 or Wayland support
    target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_XLIB_KHR)
    
    # TODO(Phase 5): Add Wayland support alongside X11
    # Details: Modern Linux distributions are moving to Wayland
    # Priority: Low
    # Dependencies: X11 version working
endif()

# Enable validation layers in debug builds
# Validation layers help catch Vulkan API misuse during development
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:VK_ENABLE_VALIDATION_LAYERS>
)

# TODO(Phase 2): Add shader compilation support
# Details: Need to compile HLSL/GLSL to SPIR-V during build
# Priority: High
# Dependencies: Basic Vulkan setup complete

# TODO(Phase 4): Add threading library support
# Details: Will need std::thread and possibly TBB for work-stealing
# Priority: Medium
# Dependencies: Phase 3 basic rendering working

# Install target for distribution
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# TODO(Phase 5): Add packaging support (CPack)
# Details: Create distributable packages for each platform
# Priority: Low
# Dependencies: All core features complete
