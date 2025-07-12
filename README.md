# Vulkan Fractal Generator

A real-time fractal generator built with C++20 and Vulkan. This project demonstrates GPU compute shaders for fractal calculations and Vulkan's graphics pipeline for rendering.

## Overview

This fractal generator uses GPU compute shaders to calculate the Mandelbrot set in real-time, then renders the results using Vulkan's graphics pipeline. The project showcases modern GPU programming techniques and provides an interactive fractal exploration tool.

## Features

- **Real-time Mandelbrot Set Generation**: Computed entirely on GPU using Vulkan compute shaders
- **Interactive GUI**: Built with Dear ImGui for parameter adjustment
  - Zoom controls (1x to 1,000,000x+ magnification)
  - Iteration count adjustment (100-1000+ iterations)
  - Center position navigation with click-and-drag
  - Color intensity scaling
- **Optimized Rendering**: Only recomputes when parameters change
- **Cross-platform Support**: Developed on macOS with MoltenVK, compatible with Windows/Linux
- **Modern C++**: Uses C++20 features throughout the codebase

## Architecture

The project is organized into several main components:

```
Application Layer
├── VulkanApplication (main coordination)
├── WindowManager (GLFW integration)
└── GuiManager (Dear ImGui integration)

Vulkan Layer
├── VulkanSetup (instance, device, queues)
├── ComputePipeline (fractal calculation)
├── GraphicsPipeline (rendering)
├── SwapchainManager (presentation)
├── ShaderManager (SPIR-V compilation)
├── MemoryManager (GPU memory)
└── TextureManager (compute-to-graphics data flow)
```

## Building and Running

### Prerequisites

- C++20 compatible compiler
- CMake 3.20 or later
- Vulkan SDK
- GLFW for windowing

### macOS

```bash
# Install dependencies via Homebrew
brew install cmake glfw vulkan-headers vulkan-loader vulkan-validationlayers molten-vk

# Build the project
git clone [your-repository-url]
cd fractal-generator
mkdir build && cd build
cmake ..
make -j

# Run
cd ..
./run.sh
```

### Windows

```cmd
# Install Vulkan SDK from LunarG
# Install GLFW (vcpkg recommended)
vcpkg install glfw3

mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### Linux

```bash
# Ubuntu/Debian
sudo apt install cmake libglfw3-dev vulkan-sdk

mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Technical Highlights

### GPU Computing
The fractal calculations are performed using Vulkan compute shaders, allowing thousands of pixels to be computed in parallel. Each compute thread calculates the Mandelbrot iteration count for a single pixel, leveraging the GPU's massive parallel processing power.

### Memory Management
The project implements custom Vulkan memory management for optimal performance:
- GPU-local memory for compute buffers
- Staging buffers for CPU-GPU data transfer
- Efficient memory type selection based on usage patterns

### Synchronization
Proper synchronization between compute and graphics operations ensures correct rendering:
- Pipeline barriers for memory dependencies
- Semaphores for queue synchronization
- Fences for CPU-GPU synchronization

## Future Enhancements

Potential improvements and additions:
- Additional fractal types (Julia sets, Burning Ship, Newton fractals)
- Multi-precision arithmetic for deeper zoom levels
- Performance profiling and optimization
- 3D fractal rendering with ray tracing

## License

This project is released under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

This project was built using:

- **Vulkan Tutorial** (vulkan-tutorial.com) - Essential learning resource
- **Sascha Willems' Vulkan Examples** - Reference implementations
- **Khronos Group** - Vulkan specification and samples
- **Dear ImGui** - GUI framework
- **GLFW** - Cross-platform windowing

---

**Note**: This is a learning project focused on understanding Vulkan concepts and GPU programming techniques.
