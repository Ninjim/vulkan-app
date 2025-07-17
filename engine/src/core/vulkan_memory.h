#pragma once
#include "vulkan_types.inl"

u32 find_memory_type(vulkan_context* context, u32 typeFilter, VkMemoryPropertyFlags properties);

void map_memory(vulkan_context* context, VkDeviceMemory memory, u64 size, u64 offset, void* src);