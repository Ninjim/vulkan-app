#pragma once
#include "vulkan_types.inl"

b8 create_image_views(vulkan_context* context, VkImageView** imageViews, VkImage* images);

// need to make the pointer point to that space in memory
b8 create_images(vulkan_context* context, VkImage** images);

// I don't think we need cleanup functions??

