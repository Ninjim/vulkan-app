#include "vulkan_swapchain.h"
#include "container/darray.h"
#include "logging.h"

b8 querySwapchainSupport(vulkan_context* context, vulkan_swapchain* swapchain){
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physicalDevice, context->surface, &swapchain->capabilities);

    // Formats
    u32 formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->physicalDevice, context->surface, &formatCount, 0);
    
    if(formatCount == 0){
        ERROR("Zero supported formats!");
        return false;
    }

    swapchain->formats = darray_reserve(VkSurfaceFormatKHR, formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->physicalDevice, context->surface, &formatCount, swapchain->formats);

    // Present modes
    u32 presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->physicalDevice, context->surface, &presentModeCount, 0);
    
    if(presentModeCount == 0){
        ERROR("Zero supported present modes!");
        return false;
    }

    swapchain->presentModes = darray_reserve(VkPresentModeKHR, presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->physicalDevice, context->surface, &presentModeCount, swapchain->presentModes);

    return true;

}







b8 swapchainCreate(vulkan_context* context, vulkan_swapchain* swapchain){
    
    // Checking format
    VkSurfaceFormatKHR availableFormat;

    b8 formatFound = false;
    for(u32 i = 0; i < darray_capacity(swapchain->formats); ++i){
        if(swapchain->formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && swapchain->formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
            availableFormat = swapchain->formats[i];
            formatFound = true;
            break;
        }
    }

    if(!formatFound){
        WARNING("Suitable format not found, picking first one");
        availableFormat = swapchain->formats[0];
    }

    // Presentation mode
    // Mailbox mode is exists, otherwise fifo
    // TODO: if the framerate is capped, no reason for mailbox, use fifo, otherwise mailbox, but just to see how well it can run, lets do mailbox
    VkPresentModeKHR presentMode;
    
    b8 modeFound = false;
    for(u32 i = 0; i < darray_capacity(swapchain->presentModes); ++i){
        if(swapchain->presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR){
            presentMode = swapchain->presentModes[i];
            modeFound = true;
            break;
        }
    }

    if(!modeFound){
        WARNING("Mailbox mode not found, using FIFO instead");
        presentMode = VK_PRESENT_MODE_FIFO_KHR; // this is guaranteed if swapchain support exists
    }

    // Swap extent
    // Vulkan uses pixels, and normal conventions use screen coordinates, because of that
    // we use a glfw function to get the screen size in pixels, in case a certain display has an above average pixel density and pixels don't correspond to screen coordinates
    VkExtent2D extent;
   
    // maximum uint value, if this is the case then the current extent of the window cannot be used for the swapchain and we need to query ourselves for the window size
    // the surface doesn't change the resolution on the fly I guess :|
    if(swapchain->capabilities.currentExtent.width != 4294967295U){
        extent = swapchain->capabilities.currentExtent;
    } else {
        glfwGetFramebufferSize(context->window, (i32*)&context->width, (i32*)&context->height);
        extent.width = CLAMP(context->width, swapchain->capabilities.minImageExtent.width, swapchain->capabilities.maxImageExtent.width);
        extent.height = CLAMP(context->height, swapchain->capabilities.minImageExtent.height, swapchain->capabilities.maxImageExtent.height);        
    }

    // Swapchain creation

    // Images
    u32 imageCount = swapchain->capabilities.minImageCount+1; // for triple buffering, in most cases min is 2 so this results in 3

    // 0 max means theres no limit, so if there is, we don't wanna go over it
    if(swapchain->capabilities.maxImageCount > 0 && imageCount > swapchain->capabilities.maxImageCount){
        imageCount = swapchain->capabilities.maxImageCount;
    }

    if(imageCount < 3 && presentMode == VK_PRESENT_MODE_MAILBOX_KHR){
        WARNING("Insufficient swapchain image count (%i) for mailbox, reverting to fifo", imageCount);
        presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }

    VkSwapchainCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    createInfo.surface = context->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = availableFormat.format;
    createInfo.imageColorSpace = availableFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // ????? for 3D images?
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // if we're rendering directly to this, color bit, if we're rendering to some other image first and then moving them to the swapchain (for post-processing maybe), then we use transfer_dst bit

    // Queues, if graphics and present are different, we need concurrent
    
    // defining it here or else by the time we use it it'll go out of scope since it's a pointer?
    u32 queueFamilyIndices[] = {context->graphicsFamily, context->presentFamily};

    if(context->graphicsFamily != context->presentFamily){
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional here
        createInfo.pQueueFamilyIndices = 0; // Optional here
    }

    createInfo.preTransform = swapchain->capabilities.currentTransform; // for no transformations, otherwise stuff like phones could rotate 90 degrees for example
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // ignore blending
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE; // if another window is on top, the color of the clipped pixels won't be rendered
    createInfo.oldSwapchain = VK_NULL_HANDLE; // for use in future, when window was resized for example

    VK_CHECK(vkCreateSwapchainKHR(context->device, &createInfo, context->allocator, &swapchain->swapchain));

    swapchain->format = availableFormat.format;
    swapchain->extent = extent;

    // Retrieve image handles, destroyed with the swapchain so no need for cleanup
    vkGetSwapchainImagesKHR(context->device, swapchain->swapchain, &imageCount, 0);
    swapchain->swapchainImages = darray_reserve(VkImage, imageCount);
    vkGetSwapchainImagesKHR(context->device, swapchain->swapchain, &imageCount, swapchain->swapchainImages);

    context->swapchain.imageCount = imageCount;

    //SUCCESS("Swapchain image handles acquired!");

    return true;
}






void swapchain_cleanup(vulkan_context* context, vulkan_swapchain* swapchain){
    if(swapchain->formats){
        darray_destroy(swapchain->formats);
        swapchain->formats = 0;
    }

    if(swapchain->presentModes){
        darray_destroy(swapchain->presentModes);
        swapchain->presentModes = 0;
    }

    if(swapchain->swapchainImages){
        darray_destroy(swapchain->swapchainImages);
        swapchain->swapchainImages = 0;
    }

    if(swapchain->swapchain){
    vkDestroySwapchainKHR(context->device, swapchain->swapchain, context->allocator);
    swapchain->swapchain = 0;
    }
}