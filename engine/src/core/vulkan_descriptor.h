#pragma once
#include "vulkan_types.inl"

b8 create_descriptor_layout(vulkan_context* context, vulkan_descriptor* descriptor, u32 binding, VkDescriptorType type, u32 descriptor_count, VkShaderStageFlags stage);

b8 create_descriptor_pool(vulkan_context* context, vulkan_descriptor* descriptor, u32 descriptor_count, VkDescriptorType type, u32 max_sets);

b8 create_descriptor_sets(vulkan_context* context, vulkan_descriptor* descriptor, u32 layout_count, VkDescriptorSetLayout* layouts);

void update_descriptor_sets_buffers(vulkan_context* context, u32 descriptorset_count, VkBuffer* buffers, VkDescriptorSet* descriptor_sets, u32 dst_binding, u32* ranges, VkDescriptorType type);
