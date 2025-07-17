#include "vulkan_types.inl"

b8 querySwapchainSupport(vulkan_context* context, vulkan_swapchain* swapchain);

b8 swapchainCreate(vulkan_context* context, vulkan_swapchain* swapchain);

void swapchain_cleanup(vulkan_context* context, vulkan_swapchain* swapchain);
