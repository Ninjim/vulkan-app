#pragma once 
#include "vulkan_types.inl"

b8 create_frambuffers(vulkan_context* context, u32 image_count, u32 width, u32 height, VkFramebuffer** framebuffers, VkImageView* imageViews, VkRenderPass* renderpass);