#pragma once
#include "vulkan_types.inl"

b8 create_graphics_pipeline(vulkan_context* context, const char* vertPath, const char* fragPath, u32 width, u32 height, VkExtent2D extent, VkPipelineLayout* pipelineLayout, VkRenderPass* renderpass, VkPipeline* pipeline);

b8 pipeline_draw(VkCommandBuffer commandbuffer, VkPipeline graphics_pipeline, u32 width, u32 height, VkExtent2D extent, u64 v_count, u64 i_count, u64 v_offset, u64 i_offset, VkBuffer* vertexBuffers, VkDeviceSize* vertexOffsets, VkDeviceSize indexOffset, u32 indices, u32 index_instances, u32 indices_offset,u32 indices_vertex_offsets, u32 indance_instance_offset);
