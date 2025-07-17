#include "vulkan_framebuffer.h"
#include "container/darray.h"

b8 create_frambuffers(vulkan_context* context, u32 image_count, u32 width, u32 height, VkFramebuffer** framebuffers, VkImageView* imageViews, VkRenderPass* renderpass){
    *framebuffers = darray_reserve(VkFramebuffer, image_count);

    for(u32 i = 0; i < image_count; ++i){
        
        // Array because we could have multiple for one framebuffer?
        #define ATTACHMENT_COUNT 1
        VkImageView attachments[ATTACHMENT_COUNT] = {
            imageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferInfo.renderPass = *renderpass;
        framebufferInfo.attachmentCount = ATTACHMENT_COUNT;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;

        VK_CHECK(vkCreateFramebuffer(context->device, &framebufferInfo, context->allocator, &((*framebuffers)[i])));
    }

    return true;
}

