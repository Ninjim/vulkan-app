#pragma once
#include "vulkan_types.inl"

b8 create_buffer(vulkan_context* context, vulkan_buffer* buffer, u64 size, VkBufferUsageFlagBits usage, VkSharingMode sharing_mode, VkMemoryPropertyFlags properties, u32* family_indices, u32 family_count);

void buffer_destroy(vulkan_context* context, vulkan_buffer* buffer);

void copy_buffer(vulkan_context* context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer commandbuffer);

void update_uniform_buffer(void* map_ptr, mat4 model, mat4 view, mat4 proj);