#include "vulkan_images.h"
#include "container/darray.h"

b8 create_image_views(vulkan_context* context, VkImageView** imageViews, VkImage* images){
    // no way this should give erroneous results right?
    // this isn't specifically for swapchain images so?
    u32 imageCount = darray_capacity(images);
    *imageViews = darray_reserve(VkImageView, imageCount);

    for(u32 i = 0; i < imageCount; ++i){
        VkImageViewCreateInfo createInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        createInfo.image = images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = context->swapchain.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // what it will be used for
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(context->device, &createInfo, context->allocator, &((*imageViews)[i])));
    }

    return true;
}
