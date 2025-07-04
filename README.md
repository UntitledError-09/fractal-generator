# Vulkan Hybrid CPU-GPU Fractal Generator

# Vulkan Hybrid CPU-GPU Fractal Generator

A cross-platform real-time fractal generator built with C++20 and Vulkan, featuring GPU-accelerated Mandelbrot computation and visualization.

## Current Status: Phase 4 Complete! 🎉

**MILESTONE ACHIEVED**: Real-time GPU fractal visualization fully operational!

### What's Working Now:
- ✅ **Complete Fractal Integration**: Compute pipeline output seamlessly displayed via graphics pipeline  
- ✅ **Real-time Mandelbrot Rendering**: Beautiful, full-screen fractal visualization at 60+ FPS
- ✅ **GPU Compute Pipeline**: Mandelbrot set calculation on GPU (12,636 bytes SPIR-V)
- ✅ **Texture Management**: Buffer-to-texture transfer with proper Vulkan layout transitions
- ✅ **Graphics Pipeline**: Fullscreen quad rendering with triangle strip topology
- ✅ **Memory Management**: Efficient GPU buffer and texture allocation (~2MB for 800x600 fractal)
- ✅ **Visual Excellence**: Vibrant HSV-based color gradients showing iteration escape times
- ✅ **Apple M2 Pro Optimized**: 32GB unified memory, all systems operational

The application now renders stunning, real-time Mandelbrot fractals with complete screen coverage and beautiful color gradients. The classic fractal geometry with main cardioid, circular bulb, and intricate boundaries is displayed perfectly.

### Recent Achievements:
- TextureManager class for compute-to-graphics data transfer
- Fixed triangle strip rendering for complete screen coverage  
- Real-time fractal visualization with proper aspect ratio
- Over 960 frames rendered successfully in performance testing
- Proper synchronization between compute and graphics queues
- Real-time rendering loop with responsive window management

## Project Goals
- **Hybrid Rendering**: Intelligent CPU-GPU work distribution for optimal performance
- **Cross-Platform**: Primary support for macOS M2, secondary for Windows/Linux
- **Modern C++**: Leveraging C++20 features like concepts and ranges
- **Educational**: Extensively commented code explaining Vulkan concepts
- **Interactive**: Real-time parameter adjustment with Dear ImGui

## Architecture Overview
```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   GLFW Window   │    │  Dear ImGui GUI  │    │ Vulkan Renderer │
│   Management    │◄──►│    Controls      │◄──►│    Pipeline     │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                                        │
                               ┌────────────────────────┼────────────────────────┐
                               │                        ▼                        │
                               │               ┌─────────────────┐               │
                               │               │ Load Balancer   │               │
                               │               │   Coordinator   │               │
                               │               └─────────────────┘               │
                               │                        │                        │
                               ▼                        ▼                        ▼
                    ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
                    │  GPU Compute    │    │ Hybrid Manager  │    │  CPU Thread     │
                    │   Shaders       │    │                 │    │     Pool        │
                    │ (Vulkan)        │    │                 │    │ (Work Stealing) │
                    └─────────────────┘    └─────────────────┘    └─────────────────┘
```

## Phase Development Plan

### ✅ Phase 1: Foundation Setup (COMPLETE!)
- [x] CMake build system setup
- [x] Basic project structure
- [x] Vulkan instance and device creation
- [x] GLFW window management
- [x] Validation layer integration
- [x] Documentation framework
- [x] macOS M2 Pro compatibility verified

### � Phase 2: Compute Pipeline Foundation (Next)
- [ ] Basic compute shader compilation
- [ ] Descriptor set management
- [ ] Memory allocation utilities
- [ ] Simple Mandelbrot compute shader

### 🔲 Phase 3: Basic Fractal Rendering
- Graphics pipeline for display
- Dear ImGui integration
- Interactive parameter controls
- Basic zoom and pan functionality

### 🔲 Phase 4: CPU Threading System
- Work-stealing thread pool
- CPU fractal computation
- Hybrid CPU-GPU coordination

### 🔲 Phase 5: Advanced Features & Optimization
- Dynamic load balancing
- Multiple fractal types
- Performance profiling
- Export functionality

## Building the Project

### Prerequisites
- **C++20 compatible compiler** (GCC 11+, Clang 13+, MSVC 2022)
- **CMake 3.20+**
- **Vulkan SDK** (latest recommended)
- **GLFW 3.3+**

### macOS Setup (Primary Platform) ✅ WORKING
```bash
# Install dependencies via Homebrew
brew install cmake glfw vulkan-headers vulkan-loader vulkan-validationlayers molten-vk

# Clone and build
git clone [repository-url]
cd fractal-generator
mkdir build && cd build
cmake ..
make -j

# Run with proper environment (use provided script)
cd ..
./run.sh
```

### Windows Setup
```cmd
# Install dependencies via vcpkg (recommended)
vcpkg install glfw3 vulkan

# Or download manually:
# - Vulkan SDK from LunarG
# - GLFW from official website

mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### Linux Setup
```bash
# Ubuntu/Debian
sudo apt install cmake libglfw3-dev vulkan-sdk

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Development Phases

### ✅ Phase 1: Foundation Setup (COMPLETE!)
- [x] CMake build system setup with shaderc integration
- [x] Basic project structure with proper RAII patterns
- [x] Vulkan instance and device creation with MoltenVK support
- [x] GLFW window management and surface creation
- [x] Validation layer integration and debug messaging
- [x] Documentation framework and progress tracking
- [x] macOS M2 Pro compatibility verified and optimized

### ✅ Phase 2: Compute Pipeline Foundation (COMPLETE!)
- [x] ShaderManager: GLSL to SPIR-V compilation system
- [x] MemoryManager: GPU memory allocation and buffer management
- [x] ComputePipeline: Complete compute pipeline infrastructure
- [x] Mandelbrot compute shader (16x16 local work groups)
- [x] Descriptor set layout and allocation system
- [x] Command pool creation and command buffer recording
- [x] Parameter buffer updates and GPU dispatch
- [x] Resource lifecycle management and cleanup
- [x] Performance validation: 1.83MB output buffer, 50x38 work groups

### ✅ Phase 3: Graphics Rendering (COMPLETE!)
- [x] Swapchain creation and management (1600x1200, triple buffering)
- [x] Graphics pipeline setup (vertex/fragment shaders)
- [x] Render pass and framebuffer creation
- [x] Fullscreen quad rendering with proper topology
- [x] Frame presentation and double buffering
- [x] Shader compilation and module management
- [x] Visual confirmation with test pattern rendering

### ✅ Phase 4: Fractal Integration (COMPLETE!)
- [x] TextureManager: Buffer-to-texture data transfer system
- [x] Compute output integration with graphics pipeline
- [x] Real-time fractal visualization with proper aspect ratio
- [x] Image layout transitions and synchronization
- [x] Triangle strip rendering fix for full screen coverage
- [x] Performance optimization: 60+ FPS sustained rendering
- [x] Visual excellence: HSV color gradients and sharp detail

### 🚀 Future Phases (Potential Extensions)
- [ ] **Phase 5: Interactive Controls** - Zoom, pan, parameter adjustment
- [ ] **Phase 6: Advanced Fractals** - Julia sets, burning ship, multi-precision
- [ ] **Phase 7: Performance Optimization** - GPU profiling, async compute
- [ ] **Phase 8: User Interface** - ImGui integration, fractal exploration tools

## Code Quality Standards
1. **Extensive Documentation**: Every function and class thoroughly commented
2. **TODO-Driven Development**: All simplifications marked for future improvement
3. **Phase-Based Implementation**: No feature jumping between phases
4. **Cross-Platform First**: Consider platform differences from the start

### Documentation Files
- [`docs/implementation_progress.md`](docs/implementation_progress.md) - Development tracking
- [`docs/design_decisions.md`](docs/design_decisions.md) - Architectural rationale
- [`docs/vulkan_concepts.md`](docs/vulkan_concepts.md) - Vulkan learning resource
- [`docs/performance_notes.md`](docs/performance_notes.md) - Performance considerations

### Contributing
This project is currently in active development. Each phase must be completed and approved before proceeding to the next. See the documentation files for current status and next steps.

## License
[License to be determined]

## Acknowledgments
- **Vulkan**: Modern graphics and compute API
- **GLFW**: Cross-platform windowing
- **Dear ImGui**: Immediate mode GUI library
- **Khronos Group**: Vulkan specification and examples

---
**Status**: Phase 4 Complete! Real-time GPU fractal visualization is fully operational. 🎉
