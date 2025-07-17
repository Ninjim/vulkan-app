#include "vulkan_buffer.h"
#include "vulkan_memory.h"
#include "vulkan_command_buffer.h"
#include "logging.h"

b8 create_buffer(vulkan_context* context, vulkan_buffer* buffer, u64 size, VkBufferUsageFlagBits usage, VkSharingMode sharing_mode, VkMemoryPropertyFlags properties, u64 offset, u32* family_indices, u32 family_count){

    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharing_mode;
    bufferInfo.flags = 0;
    
    if(sharing_mode & VK_SHARING_MODE_CONCURRENT){
        bufferInfo.queueFamilyIndexCount = family_count;
        bufferInfo.pQueueFamilyIndices = family_indices;
    }

    VK_CHECK(vkCreateBuffer(context->device, &bufferInfo, context->allocator, &buffer->buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context->device, buffer->buffer, &memRequirements);

    u32 mem_index = find_memory_type(context, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if(mem_index == UINT32_MAX){
        ERROR("Could not find proper memory type for buffer creation")
        return false;
    }

    VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = mem_index;

    VK_CHECK(vkAllocateMemory(context->device, &allocInfo, context->allocator, &buffer->bufferMemory));

    vkBindBufferMemory(context->device, buffer->buffer, buffer->bufferMemory, 0);

    buffer->size = size;

    return true;
}



void copy_buffer(vulkan_context* context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkCommandBuffer commandbuffer){

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandbuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}




void buffer_destroy(vulkan_context* context, vulkan_buffer* buffer){

    if(buffer->buffer){
    vkDestroyBuffer(context->device, buffer->buffer, context->allocator);
    buffer->buffer = 0;    
    }

    if(buffer->bufferMemory){
    vkFreeMemory(context->device, buffer->bufferMemory, context->allocator);
    buffer->bufferMemory = 0;
    }
}