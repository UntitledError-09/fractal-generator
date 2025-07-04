/**
 * @file SwapchainManager.cpp
 * @brief Implementation of Vulkan swapchain management
 * 
 * @author Fractal Generator Project
 * @date July 4, 2025
 * @version Phase 3
 */

#include "SwapchainManager.h"
#include "WindowManager.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <limits>

SwapchainManager::SwapchainManager(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    const WindowManager& windowManager
)
    : m_device(device)
    , m_physicalDevice(physicalDevice)
    , m_surface(surface)
    , m_windowManager(windowManager)
    , m_swapchain(VK_NULL_HANDLE)
    , m_format(VK_FORMAT_UNDEFINED)
    , m_extent{0, 0}
{
    std::cout << "[SwapchainManager] Initialized swapchain manager" << std::endl;
}

SwapchainManager::~SwapchainManager() {
    std::cout << "[SwapchainManager] Cleaning up swapchain resources..." << std::endl;
    cleanupSwapchain();
}

bool SwapchainManager::createSwapchain() {
    std::cout << "[SwapchainManager] Creating swapchain..." << std::endl;
    
    try {
        SwapchainSupportDetails swapchainSupport = querySwapchainSupport();
        
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);
        
        // Store chosen format and extent
        m_format = surfaceFormat.format;
        m_extent = extent;
        
        // Choose number of images (prefer triple buffering)
        uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
        if (swapchainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapchainSupport.capabilities.maxImageCount) {
            imageCount = swapchainSupport.capabilities.maxImageCount;
        }
        
        // Create swapchain
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        
        VkResult result = vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain! Vulkan error: " + std::to_string(result));
        }
        
        // Get swapchain images
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
        m_images.resize(imageCount);
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_images.data());
        
        // Create image views
        createImageViews();
        
        std::cout << "[SwapchainManager] Swapchain created successfully:" << std::endl;
        std::cout << "  Format: " << m_format << std::endl;
        std::cout << "  Extent: " << m_extent.width << "x" << m_extent.height << std::endl;
        std::cout << "  Images: " << imageCount << std::endl;
        std::cout << "  Present mode: " << presentMode << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[SwapchainManager] Failed to create swapchain: " << e.what() << std::endl;
        return false;
    }
}

bool SwapchainManager::recreateSwapchain() {
    std::cout << "[SwapchainManager] Recreating swapchain..." << std::endl;
    
    // Wait for device to be idle
    vkDeviceWaitIdle(m_device);
    
    // Clean up old swapchain
    cleanupSwapchain();
    
    // Create new swapchain
    return createSwapchain();
}

VkResult SwapchainManager::acquireNextImage(VkSemaphore semaphore, uint32_t& imageIndex) {
    return vkAcquireNextImageKHR(
        m_device,
        m_swapchain,
        std::numeric_limits<uint64_t>::max(),  // No timeout
        semaphore,
        VK_NULL_HANDLE,
        &imageIndex
    );
}

VkResult SwapchainManager::presentImage(VkQueue presentQueue, uint32_t imageIndex, VkSemaphore waitSemaphore) {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;
    
    VkSwapchainKHR swapchains[] = {m_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;
    
    return vkQueuePresentKHR(presentQueue, &presentInfo);
}

SwapchainSupportDetails SwapchainManager::querySwapchainSupport() {
    SwapchainSupportDetails details;
    
    // Get surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &details.capabilities);
    
    // Get surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, details.formats.data());
    }
    
    // Get present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, details.presentModes.data());
    }
    
    return details;
}

VkSurfaceFormatKHR SwapchainManager::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    // Prefer BGRA8 SRGB for optimal performance and color accuracy
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    
    // Fallback to first available format
    return availableFormats[0];
}

VkPresentModeKHR SwapchainManager::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    // Prefer mailbox mode for low latency triple buffering
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    
    // FIFO is guaranteed to be available
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapchainManager::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        // Get window dimensions
        int width, height;
        m_windowManager.getFramebufferSize(width, height);
        
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        
        // Clamp to supported range
        actualExtent.width = std::clamp(actualExtent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);
        
        return actualExtent;
    }
}

void SwapchainManager::createImageViews() {
    m_imageViews.resize(m_images.size());
    
    for (size_t i = 0; i < m_images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        
        VkResult result = vkCreateImageView(m_device, &createInfo, nullptr, &m_imageViews[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view! Vulkan error: " + std::to_string(result));
        }
    }
}

void SwapchainManager::cleanupSwapchain() {
    // Destroy image views
    for (auto imageView : m_imageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }
    m_imageViews.clear();
    
    // Destroy swapchain
    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
    
    // Clear image handles (they're owned by the swapchain)
    m_images.clear();
}
