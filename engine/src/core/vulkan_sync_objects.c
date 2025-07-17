#include "vulkan_sync_objects.h"

b8 create_semaphores(vulkan_context* context, u32 count, VkSemaphore* semaphores){
    VkSemaphoreCreateInfo semaphoreInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    
    for(u32 i = 0; i < count; ++i){
    vkCreateSemaphore(context->device, &semaphoreInfo, context->allocator, &semaphores[i]);
    }

    return true;
}

b8 create_fences(vulkan_context* context, u32 count, VkFence* fences){
    VkFenceCreateInfo fenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // so the start of the frame doesn't block indefinitely
    
    for(u32 i = 0; i < count; ++i){
    vkCreateFence(context->device, &fenceInfo, context->allocator, &fences[i]);
    }

    return true;
}