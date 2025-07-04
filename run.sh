#!/bin/bash
# 
# Run script for Vulkan Fractal Generator on macOS
# This script sets up the proper environment variables for MoltenVK and Vulkan validation layers
#

echo "=== Vulkan Fractal Generator Run Script ==="
echo "Setting up MoltenVK environment for macOS..."

# Set up MoltenVK ICD path
export VK_ICD_FILENAMES=/opt/homebrew/opt/molten-vk/etc/vulkan/icd.d/MoltenVK_icd.json

# Set up Vulkan validation layers path
export VK_LAYER_PATH=/opt/homebrew/opt/vulkan-validationlayers/share/vulkan/explicit_layer.d

echo "Environment configured:"
echo "  VK_ICD_FILENAMES=$VK_ICD_FILENAMES"
echo "  VK_LAYER_PATH=$VK_LAYER_PATH"
echo ""

# Change to project root directory
cd "$(dirname "$0")"

if [ ! -f "./build/VulkanFractalGenerator" ]; then
    echo "Error: VulkanFractalGenerator not found. Please build the project first:"
    echo "  mkdir -p build && cd build"
    echo "  cmake .."
    echo "  make -j"
    exit 1
fi

echo "Running Vulkan Fractal Generator..."
echo "Close the window or press Ctrl+C to exit."
echo ""

./build/VulkanFractalGenerator
