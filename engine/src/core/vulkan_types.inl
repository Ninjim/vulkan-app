#pragma once

#include "defines.h"
#define GLFW_INCLUDE_VULKAN
#include "vulkan_utils.h"
#include "../include/glfw/glfw3.h"



// Buffers
typedef struct vulkan_buffer {
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    VkDeviceSize size;
    u32 offsets[4];
} vulkan_buffer;



// Command buffers
typedef struct vulkan_commands {
    VkCommandPool commandPool;
    VkCommandBuffer* commandbuffers;
} vulkan_commands;


// Swapchain
typedef struct vulkan_swapchain {

VkSurfaceCapabilitiesKHR capabilities;
VkSurfaceFormatKHR* formats;
VkPresentModeKHR* presentModes;
VkSwapchainKHR swapchain;

VkFormat format;
VkExtent2D extent;

// Images
VkImage* swapchainImages;
VkImageView* swapchainImageViews;
u32 imageCount;

// Framebuffers
VkFramebuffer* swapchainFramebuffers;

// Pipeline layout??
VkPipelineLayout pipelineLayout;
VkRenderPass renderpass;
VkPipeline graphicsPipeline;

// Sync objects
VkSemaphore* imageAvailableSemaphore;
VkSemaphore* renderFinishedSemaphore;
VkFence* inFlightFence;

} vulkan_swapchain;


// Context
typedef struct vulkan_context {

// GLFW
GLFWwindow* window;
u32 width;
u32 height;
GLFWmonitor* primaryMonitor;
const GLFWvidmode* mode;

// Instance
VkInstance instance;
const VkAllocationCallbacks* allocator;

// DEBUG
#ifdef _DEBUG
VkDebugUtilsMessengerEXT debugMessenger;
#endif 

// Physical device
VkPhysicalDevice physicalDevice;
u32 graphicsFamily;
u32 computeFamily;
u32 transferFamily;
u32 presentFamily; // drawing queues can possibly not be able to present
#define EXTENSION_COUNT 1
const char* requiredExtensions[EXTENSION_COUNT];

// Logical device
VkDevice device;
VkQueue graphicsQueue; // implicitly cleaned up at device destruction
VkQueue presentQueue;
VkQueue transferQueue;

// Buffers
vulkan_buffer vertexIndexBuffer;
vulkan_buffer vertexIndexBufferStaging;

// Surface
VkSurfaceKHR surface;

// Swapchain
vulkan_swapchain swapchain;

// Command pool / buffers
vulkan_commands graphics_command_buffer;
vulkan_commands transfer_command_buffer;

// Frame
u32 currentFrame;

// Window resize
b8 resize;

} vulkan_context;
