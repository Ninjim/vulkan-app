#include "vulkan_memory.h"
#include <string.h>

u32 find_memory_type(vulkan_context* context, u32 typeFilter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(context->physicalDevice, &memProperties);

    for(u32 i = 0; i < memProperties.memoryTypeCount; ++i){
        if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties){
            return i;
        }
    }

    return UINT32_MAX; // invalid
}

void map_memory(vulkan_context* context, VkDeviceMemory memory, u64 size, u64 offset, void* src){
    void* data;
    vkMapMemory(context->device, memory, offset, size, 0, &data);
    memcpy(data, src, size);
    vkUnmapMemory(context->device, memory);
}