#include "vulkan_renderpass.h"

b8 create_renderpass(vulkan_context* context, VkFormat attachment_format, VkRenderPass* renderpass){

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = attachment_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // to clear the prev contents
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // why no depth?

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // i could probably get away with VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL for right now
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // presentable by swapchain

    // Attachment references
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0; // which index in the attachment description array this ref references (by index, so 0)
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // what layout to transition the attachment during the subpass

    // Subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // i dont think theres compute support yet?
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // the index of the attachment in the array corresponds to the OUT LOCATION variable index in the fragment shader???
    //theres input, resolve (multisampling), depth/stencil and preserve attachments too

    // TODO: Dependency, probably wanna make this like a custom thing or something?
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // commands submitted before renderpass begins, technically we just use this cuz we only have one subpass
    dependency.dstSubpass = 0; // this subpass, the index of the subpass we want to use in the subpass array
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // where the access operation to wait on until it finishes occurs
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // where the wait actually happens?
    dependency.srcAccessMask = 0; // if it is zero then that probably just means wait until the operations reach the stage mentioned by the stage mask?
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // the operation we want to wait on until the operation we need to finish (in the src) finishes, in this case writing the results of the color output stage to the color attachment

    // Create info
    VkRenderPassCreateInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VK_CHECK(vkCreateRenderPass(context->device, &renderPassInfo, context->allocator, renderpass))
    

    return true;
}


void start_renderpass(VkCommandBuffer commandbuffer, VkRenderPass renderpass, VkFramebuffer framebuffer, VkExtent2D extent){
    VkRenderPassBeginInfo renderPassInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassInfo.renderPass = renderpass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = extent;
    
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}




void end_renderpass(VkCommandBuffer commandbuffer){
    vkCmdEndRenderPass(commandbuffer);
}