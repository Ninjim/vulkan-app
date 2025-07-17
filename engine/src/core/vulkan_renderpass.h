#pragma once
#include "vulkan_types.inl"

// TODO: expand for multiple attachments
b8 create_renderpass(vulkan_context* context, VkFormat attachment_format, VkRenderPass* renderpass);

void start_renderpass(VkCommandBuffer commandbuffer, VkRenderPass renderpass, VkFramebuffer framebuffer, VkExtent2D extent);

void end_renderpass(VkCommandBuffer commandbuffer);