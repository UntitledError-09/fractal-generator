# Design Decisions

This document outlines the key architectural and implementation decisions made during the development of this Vulkan fractal generator.

## Development Approach

### Phase-Based Development
**Decision**: Implement the project in 5 distinct phases
**Rationale**: 
- Vulkan's complexity benefits from incremental development
- Each phase builds stable foundations for the next
- Allows for iterative learning and design refinement
- Prevents scope creep and maintains project focus

### Language Choice: C++20
**Decision**: Use C++20 features throughout the codebase
**Rationale**:
- Concepts improve template error messages and API clarity
- Ranges simplify data processing pipelines
- Modern features enhance code readability and maintainability
- Provides good learning opportunity for contemporary C++

## Vulkan Implementation Decisions

### Memory Management Strategy
**Decision**: Use explicit memory management with custom allocators
**Rationale**:
- Educational value in understanding Vulkan's memory model
- Better control over performance characteristics
- Foundation for advanced optimization techniques
**Implementation**: 
- Phase 1: Basic buffer/image creation utilities
- Phase 2: Memory pool management
- Phase 4: Advanced allocation strategies

### Queue Family Usage
**Decision**: Use separate queues for graphics and compute when available
**Rationale**:
- Enables concurrent graphics and compute operations
- Better utilizes modern GPU architectures
- Provides learning experience with queue management
**Implementation**:
- Detect separate compute queues during device selection
- Fall back to unified queue if necessary
- Implement proper synchronization between queues

### Validation Layers
**Decision**: Enable validation layers in debug builds only
**Rationale**:
- Essential for learning and debugging Vulkan
- Performance impact acceptable during development
- Production builds remain clean and optimized

### Descriptor Set Management
**Decision**: Implement custom descriptor pool management
**Rationale**:
- Educational value in understanding descriptor lifecycle
- Better control over allocation patterns
- Foundation for dynamic descriptor updates in complex rendering

## Platform and Framework Decisions

### Primary Development Platform: macOS
**Decision**: Develop primarily on macOS with MoltenVK
**Rationale**:
- MoltenVK provides good Vulkan compatibility on macOS
- Apple Silicon represents modern hardware architecture
- Good test case for non-native Vulkan support
**Considerations**: MoltenVK limitations may require workarounds

### Cross-Platform Strategy
**Decision**: Design for cross-platform from the start
**Rationale**:
- Easier to design portably than retrofit
- CMake provides good abstraction
- Vulkan specification ensures API consistency
**Implementation**:
- Platform-specific code isolated in CMake
- Abstract interfaces for platform differences
- Regular testing on secondary platforms

### GUI Framework: Dear ImGui
**Decision**: Use Dear ImGui for parameter controls
**Rationale**:
- Excellent Vulkan integration available
- Immediate mode suits parameter tweaking workflow
- Lightweight and performant
- Good documentation and community support

### Build System: Modern CMake
**Decision**: Modern CMake with target-based configuration
**Rationale**:
- Better dependency management
- Cleaner platform-specific handling
- Industry standard for C++ projects
- Good cross-platform support

### Shader Compilation: Build-time SPIR-V
**Decision**: Compile shaders to SPIR-V during CMake build
**Rationale**:
- Ensures shaders match application version
- Better error checking at build time
- Easier distribution without runtime compilation dependencies

## Error Handling and Threading

### Error Handling Strategy
**Decision**: Exception-based error handling with Vulkan result checking
**Rationale**:
- C++ exceptions provide clean error propagation
- Vulkan result codes must be checked consistently
- RAII ensures proper cleanup during errors
**Implementation**:
- Custom exception types for different error categories
- Macro for Vulkan result checking
- Proper resource cleanup in destructors

### Threading Model
**Decision**: Work-stealing thread pool for CPU computation
**Rationale**:
- Efficient load balancing for irregular workloads
- Scales well with available CPU cores
- Integrates well with GPU synchronization
