#include "vulkan_command_buffer.h"
#include "container/darray.h"

b8 create_command_buffer_pool(vulkan_context* context, u32 queueIndex, vulkan_commands* commands_struct){
    VkCommandPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // so we can individually reset command buffers
    poolInfo.queueFamilyIndex = queueIndex;
    
    VK_CHECK(vkCreateCommandPool(context->device, &poolInfo, context->allocator, &commands_struct->commandPool));

    return true;
}





b8 create_command_buffers(vulkan_context* context, VkCommandPool pool, VkCommandBuffer* commandbuffers, u32 buffer_count){
    
    VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = buffer_count;

    VK_CHECK(vkAllocateCommandBuffers(context->device, &allocInfo, commandbuffers));

    return true;
}



void record_command_buffer(VkCommandBuffer commandbuffer){

    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = 0;

    VK_CHECK(vkBeginCommandBuffer(commandbuffer, &beginInfo));
}


void record_command_buffer_one_time(VkCommandBuffer commandbuffer){

    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = 0;

    VK_CHECK(vkBeginCommandBuffer(commandbuffer, &beginInfo));
}


void end_command_buffer(VkCommandBuffer commandbuffer){
    vkEndCommandBuffer(commandbuffer);
}



void reset_command_buffer(VkCommandBuffer commandbuffer){
    vkResetCommandBuffer(commandbuffer, 0);
}



void free_command_buffers(vulkan_context* context, VkCommandPool pool, u32 buffer_count, VkCommandBuffer* commandbuffers){
    vkFreeCommandBuffers(context->device, pool, buffer_count, commandbuffers);
}


void create_and_begin_command_buffer(vulkan_context* context, VkCommandPool pool, VkCommandBuffer* commandbuffer){
    create_command_buffers(context, pool, commandbuffer, 1);
    record_command_buffer_one_time(*commandbuffer);
}


void end_reset_command_buffer(vulkan_context* context, VkCommandBuffer commandbuffer){
    end_command_buffer(commandbuffer);
    reset_command_buffer(commandbuffer);
}

void end_reset_free_command_buffer(vulkan_context* context, VkCommandBuffer* commandbuffer, VkCommandPool pool){
    end_command_buffer(*commandbuffer);
    reset_command_buffer(*commandbuffer);
    free_command_buffers(context, pool, 1, commandbuffer);
}

void command_buffers_cleanup(vulkan_context* context, vulkan_commands* commands_struct){

    if(commands_struct->commandPool){
    vkDestroyCommandPool(context->device, commands_struct->commandPool, context->allocator);
    commands_struct->commandPool = 0;
    }

    if(commands_struct->commandbuffers){
        darray_destroy(commands_struct->commandbuffers);
        commands_struct->commandbuffers = 0;
    }
}


