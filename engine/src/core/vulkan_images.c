#include "vulkan_images.h"
#include "container/darray.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb/stb_image.h"
#include "vulkan_buffer.h"
#include "vulkan_memory.h"

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




b8 create_texture_image(vulkan_context* context, const char* file_path, vulkan_image* image){

    int width, height, channels;
    u8* pixels = stbi_load(file_path, &width, &height, &channels, STBI_rgb_alpha);
    VkDeviceSize image_size = width * height * channels;

    if(!pixels){
        ERROR("Could not load image");
        return false;
    }

    // Staging image buffer
    vulkan_buffer staging_buffer;
    if(!create_buffer(context, &taging_buffer, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PPERTY_HOST_VISIBLE_BIT, 0, 0)){
        ERROR("Could not create staging buffer");
        return false;
    }

    vkMapMemory(context->device, staging_buffer.bufferMemory, 0, image_size, 0, &staging_buffer.map_ptr);
    memcpy(staging_buffer.map_ptr, pixels, image_size);
    vkUnmapMemory(context->device, staging_buffer.bufferMemory);

    stbi_image_free(pixels);

    VkImageCreateInfo image_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1; // 3d images
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // because this is a texture we want to sample from
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags = 0;

    VK_CHECK(vkCreateImage(context->device, &image_info, context->allocator, &image->image));

    // Memory
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(context->device, image->image, &mem_requirements);
    u32 mem_index = find_memory_type(context, mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMemoryAllocateInfo mem_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    mem_info.allocationSize = mem_requirements.size;
    mem_info.memoryTypeIndex = mem_index;

    VK_CHECK(vkAllocateMemory(context->device, &mem_info, context->allocator, &image->imageMemory));

    vkBindImageMemory(context->device, image->image, image->imageMemory, 0);


    return true;
}