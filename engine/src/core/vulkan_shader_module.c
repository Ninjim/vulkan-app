#include "vulkan_shader_module.h"
#include "filesystem.h"
#include "logging.h"
#include <stdlib.h>

b8 create_shader_module(vulkan_context* context, const char* file_path, VkShaderModule* shaderModule){
    file_handle handle;
    if(!filesystem_open(file_path, FILE_MODE_READ, true, &handle)){
        ERROR("File could not be found")
        return false;
    }
    
    u8* code;
    u64 bytes_read = 0;
    if(!filesystem_read_all_bytes(&handle, &code, &bytes_read)){
        ERROR("Could not read file byte-code")
        return false;
    } else if(bytes_read == 0){
        ERROR("No bytes read")
        return false;
    }

    VkShaderModuleCreateInfo createInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    createInfo.pCode = (u32*)code; // hopefully this works??
    createInfo.codeSize = bytes_read;

    VK_CHECK(vkCreateShaderModule(context->device, &createInfo, context->allocator, shaderModule));

    // cleanup
    filesystem_close(&handle);
    free(code);

    return true;
}