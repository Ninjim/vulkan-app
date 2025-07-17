#pragma once

#include "logging.h"
#include "vulkan_types.inl"
#include "container/darray.h"

b8 run();

b8 initWindow(vulkan_context* context);

b8 initVulkan(vulkan_context* context);

b8 mainLoop(vulkan_context* context);

void cleanup(vulkan_context* context);