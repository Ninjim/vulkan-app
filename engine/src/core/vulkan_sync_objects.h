#pragma once
#include "vulkan_types.inl"

b8 create_semaphores(vulkan_context* context, u32 count, VkSemaphore* semaphores);

b8 create_fences(vulkan_context* context, u32 count, VkFence* fences);