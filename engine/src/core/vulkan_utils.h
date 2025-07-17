#pragma once

#include "defines.h"
#include "asserts.h"
#include <vulkan/vulkan.h>

#define VK_CHECK(expr){ \
    VkResult result = expr; \
    ASSERT(result == VK_SUCCESS); \
}

const char* vulkan_result_string(VkResult result, b8 get_extended);

b8 vulkan_result_is_success(VkResult result); 
