#pragma once
#include "vulkan_types.inl"

b8 create_shader_module(vulkan_context* context, const char* file_path, VkShaderModule* shaderModule);