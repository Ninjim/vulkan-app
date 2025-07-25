#include "vulkan_descriptor.h"

b8 create_descriptor_layout(vulkan_context* context, vulkan_descriptor* descriptor, u32 binding, VkDescriptorType type, u32 descriptor_count, VkShaderStageFlags stage){

    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = binding;
    uboLayoutBinding.descriptorType = type;
    uboLayoutBinding.descriptorCount = descriptor_count;
    uboLayoutBinding.stageFlags = stage;
    uboLayoutBinding.pImmutableSamplers = 0; // multisampling

    VkDescriptorSetLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VK_CHECK(vkCreateDescriptorSetLayout(context->device, &layoutInfo, context->allocator, &descriptor->layout));

    return true;
}





b8 create_descriptor_pool(vulkan_context* context, vulkan_descriptor* descriptor, u32 descriptor_count, VkDescriptorType type, u32 max_sets){

    VkDescriptorPoolSize poolSize = {};
    poolSize.type = type;
    poolSize.descriptorCount = descriptor_count;

    VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets = max_sets;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    //poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VK_CHECK((vkCreateDescriptorPool(context->device, &poolInfo, context->allocator, &context->ubo_descriptor.pool)));

    return true;
}




b8 create_descriptor_sets(vulkan_context* context, vulkan_descriptor* descriptor, u32 layout_count, VkDescriptorSetLayout* layouts){

    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = descriptor->pool;
    allocInfo.descriptorSetCount = layout_count;
    allocInfo.pSetLayouts = layouts;

    VK_CHECK(vkAllocateDescriptorSets(context->device, &allocInfo, context->ubo_descriptor.sets));

    return true;
}





void update_descriptor_sets_buffers(vulkan_context* context, u32 descriptorset_count, VkBuffer* buffers, VkDescriptorSet* descriptor_sets, u32 dst_binding, u32* ranges, VkDescriptorType type){

    for(u32 i = 0; i < descriptorset_count; ++i){
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = buffers[i];
        bufferInfo.range = ranges[i];
        
        VkWriteDescriptorSet descriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptorWrite.dstSet = descriptor_sets[i];
        descriptorWrite.dstBinding = dst_binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = type;
        descriptorWrite.descriptorCount = 1;

        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = 0;
        descriptorWrite.pTexelBufferView = 0;

        vkUpdateDescriptorSets(context->device, 1, &descriptorWrite, 0, 0);
    }

}