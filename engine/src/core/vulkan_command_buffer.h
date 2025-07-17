#pragma once
#include "vulkan_types.inl"

b8 create_command_buffer_pool(vulkan_context* context, u32 queueIndex, vulkan_commands* commands_struct);

b8 create_command_buffers(vulkan_context* context, VkCommandPool pool, VkCommandBuffer* commandbuffers, u32 buffer_count);

void record_command_buffer(VkCommandBuffer commandbuffer);

void record_command_buffer_one_time(VkCommandBuffer commandbuffer);

void end_command_buffer(VkCommandBuffer commandbuffer);

void command_buffers_cleanup(vulkan_context* context, vulkan_commands* commands_struct);

void end_command_buffer(VkCommandBuffer commandbuffer);

void reset_command_buffer(VkCommandBuffer commandbuffer);

void free_command_buffers(vulkan_context* context, VkCommandPool pool, u32 buffer_count, VkCommandBuffer* commandbuffers);

void create_and_begin_command_buffer(vulkan_context* context, VkCommandPool pool, VkCommandBuffer* commandbuffer);

void end_reset_command_buffer(vulkan_context* context, VkCommandBuffer commandbuffer);

void end_reset_free_command_buffer(vulkan_context* context, VkCommandBuffer* commandbuffer, VkCommandPool pool);