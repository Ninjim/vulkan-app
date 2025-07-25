#include "vulkan_app.h"
#include "vulkan_swapchain.h"
#include "vulkan_images.h"
#include "vulkan_renderpass.h"
#include "vulkan_pipeline.h"
#include "vulkan_framebuffer.h"
#include "vulkan_command_buffer.h"
#include "vulkan_sync_objects.h"
#include "math/math_types.h"
#include "math/kmath.h"
#include "vulkan_buffer.h"
#include "vulkan_memory.h"
#include "vulkan_descriptor.h"


static vulkan_context context;

// Helper declarations
b8 createInstance(vulkan_context* context);

b8 pickPhysicalDevice(vulkan_context* context);

b8 isDeviceSuitable(VkPhysicalDevice device);

b8 createLogicalDevice(vulkan_context* context);

b8 drawFrame(vulkan_context* context, vulkan_swapchain* swapchain);

b8 resize_swapchain(vulkan_context* context, vulkan_swapchain* swapchain);


// Debugger

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

        if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT){
        ERROR("%s", pCallbackData->pMessage);
        }
   
        else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT){
        WARNING("%s", pCallbackData->pMessage);
        }
        
        else if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT){
        LOG("%s", pCallbackData->pMessage);
        }

        else if(messageSeverity ==VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT){
        TRACE("%s", pCallbackData->pMessage);
        }

        else{
        LOG("%s", pCallbackData->pMessage);
        }


    return VK_FALSE;
}





// GLFW resize callback
void resizeCallback(GLFWwindow* window, i32 width, i32 height){
vulkan_context* v_context = (vulkan_context*)(glfwGetWindowUserPointer(window));
if(v_context->width != width || v_context->height != height){
v_context->resize = true;
}
}






b8 run() {

    // Context setup
    memset(&context, 0, sizeof(vulkan_context));
    context.width = 800;
    context.height = 600;
    context.currentFrame = 0;
    context.resize = false;

    // Init window
     if(!initWindow(&context)){
        ERROR("Failure initializing window");
        return false;
    }

    SUCCESS("Window initialized successfully!");

    // Init vulkan
    if(!initVulkan(&context)){
        ERROR("Failure initializing Vulkan");
        return false;
    }

    SUCCESS("Vulkan initialized successfully!");

    // Game loop
    if(!mainLoop(&context)){
        ERROR("Gameloop failure");
        return false;
    }

    // Cleanup
    cleanup(&context);

    return true;
}






b8 initWindow(vulkan_context* context){

    if(!glfwInit()){
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); //for borderless fullscreen, need to recreate window

    context->primaryMonitor = glfwGetPrimaryMonitor(); // for fullscreen, where we use the monitor and fullscreen resolution
    context->mode = glfwGetVideoMode(context->primaryMonitor);

    context->window = glfwCreateWindow(context->width, context->height, "Vulkan", 0, 0);
    glfwSetWindowUserPointer(context->window, context);
    glfwSetFramebufferSizeCallback(context->window, resizeCallback);

    return true;
}




b8 mainLoop(vulkan_context* context){
    while(!glfwWindowShouldClose(context->window)){
        glfwPollEvents();
        if(!drawFrame(context, &context->swapchain)){
            ERROR("Error during drawFrame()");
            break;
        }
    }

    vkDeviceWaitIdle(context->device);

    return true;
}


// Draw func
b8 drawFrame(vulkan_context* context, vulkan_swapchain* swapchain){
    vkWaitForFences(context->device, 1, &(swapchain->inFlightFence[context->currentFrame]), VK_TRUE, UINT64_MAX);
    // Acquire swapchain image index
    u32 imageIndex;
    VkResult image_result = vkAcquireNextImageKHR(context->device, swapchain->swapchain, UINT64_MAX, swapchain->imageAvailableSemaphore[context->currentFrame], VK_NULL_HANDLE, &imageIndex);

    // we dont pass resize check here because it could be that we do acquire the image, and the result is success, but we do resize, therefore the semaphore is signaled, we boot out, and there is no command to unsignal it, so the present command fails
    if(image_result == VK_ERROR_OUT_OF_DATE_KHR){
        if(!resize_swapchain(context, swapchain)){
            ERROR("Failure recreating swapchain");
            return false;
        } // should work?

        context->resize = false;
        SUCCESS("Swapchain resized!");
        return true; // so we draw on the next call since all the info is out of date
    } else if (image_result != VK_SUCCESS && image_result != VK_SUBOPTIMAL_KHR){
        ERROR("Failred to aqcuire image!");
        return false;
    }

    vkResetFences(context->device, 1, &swapchain->inFlightFence[context->currentFrame]); // reset the fence if we're continuing

    // Pos calculations 
    static f32 increment = 0.01f;
    increment = increment + 0.001f;
    mat4 model = quat_to_mat4(quat_from_axis_angle((vec3){0.0f, 0.0f, 1.0f}, increment * deg_to_rad(90.0f), true));
    //mat4 translate = mat4_translation((vec3){1.0f, 1.0f, 1.0f});
    //model = mat4_mul(translate, model);
    mat4 view = mat4_look_at((vec3){2.0f, 2.0f, 2.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 1.0f});
    //mat4 view = mat4_identity();
    mat4 proj = mat4_perspective(deg_to_rad(45.0f), swapchain->extent.width / (float)swapchain->extent.height, 0.01f, 10000.0f);
    //proj.data[5] *= -1.0f;
    //mat4 proj = mat4_identity();
    update_uniform_buffer(context->uniformBuffers[context->currentFrame].map_ptr, model, view, proj);

    // Vertex buffer and offsets and descriptors
    VkBuffer ubo_buffers[] = {
        context->uniformBuffers[context->currentFrame].buffer
    };

    VkDescriptorSet descriptorsets[] = {
        context->ubo_descriptor.sets[context->currentFrame]
    };

    u32 ranges[] = {
        context->uniformBuffers[context->currentFrame].size
    };
    update_descriptor_sets_buffers(context, 1, ubo_buffers, descriptorsets, 0, ranges, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    
    VkBuffer buffers[] = {
        context->vertexIndexBuffer.buffer
    };

    VkDeviceSize offsets[] = {0};


    // Recording commands
    reset_command_buffer(context->graphics_command_buffer.commandbuffers[context->currentFrame]);
    record_command_buffer(context->graphics_command_buffer.commandbuffers[context->currentFrame]);
    
    start_renderpass(context->graphics_command_buffer.commandbuffers[context->currentFrame], swapchain->renderpass, swapchain->swapchainFramebuffers[imageIndex], swapchain->extent);
    pipeline_draw(context->graphics_command_buffer.commandbuffers[context->currentFrame], 
        swapchain->graphicsPipeline,
        swapchain->pipelineLayout,
        swapchain->extent.width,
        swapchain->extent.height,
        swapchain->extent,
        descriptorsets,
        1,
        (context->vertexIndexBufferStaging.size/sizeof(vertex_3d)) , 1, 0, 0,
        buffers,
        offsets,
        context->vertexIndexBuffer.offsets[0],
        ((context->vertexIndexBufferStaging.size - context->vertexIndexBufferStaging.offsets[0])/sizeof(u16)),
        1, 0, 0, 0);
    end_renderpass(context->graphics_command_buffer.commandbuffers[context->currentFrame]);
    end_command_buffer(context->graphics_command_buffer.commandbuffers[context->currentFrame]); // here I think??

    // Submit info
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    #define IMAGE_SEMAPHORES 1
    VkSemaphore imageAvailable[] = {
        swapchain->imageAvailableSemaphore[context->currentFrame]
    };

    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT // wait until the color is output into the image
    }; // the indexes of both arrays correspond to each other

    submitInfo.waitSemaphoreCount = IMAGE_SEMAPHORES;
    submitInfo.pWaitSemaphores = imageAvailable;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &context->graphics_command_buffer.commandbuffers[context->currentFrame];

    #define RENDER_SEMAPHORES 1
    VkSemaphore renderFinish[] = {
        swapchain->renderFinishedSemaphore[imageIndex] // because we cant wait on fences for presentation, we dont know when presentation is done for which semaphore, so we need to go off of image index to be sure!
    };
    submitInfo.signalSemaphoreCount = RENDER_SEMAPHORES;
    submitInfo.pSignalSemaphores = renderFinish;

    vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, swapchain->inFlightFence[context->currentFrame]);

    // Presentation
    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = RENDER_SEMAPHORES;
    presentInfo.pWaitSemaphores = renderFinish;
    
    #define SWAPCHAIN_COUNT 1
    VkSwapchainKHR swapchains[] = {
        swapchain->swapchain
    };
    presentInfo.swapchainCount = SWAPCHAIN_COUNT;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = 0; // in case we use multiple swapchains and want to check the results of each one, the present function's return value is indicated by the success of only the first swapchain


    VkResult present_result = vkQueuePresentKHR(context->presentQueue, &presentInfo);

    if(present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR || context->resize){
        if(!resize_swapchain(context, swapchain)){
            ERROR("Failure recreating swapchain");
            context->currentFrame = (context->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
            return false;
        } // should work?

        context->resize = false;
        SUCCESS("Swapchain resized!");
        // no return because we want to update the current frame still and we're at the end
    } else if (present_result != VK_SUCCESS){
        ERROR("Failed to present image!");
        context->currentFrame = (context->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT; // just in case?
        return false;
    }

    context->currentFrame = (context->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return true;
}



void cleanup(vulkan_context* context){

    // Vulkan cleanup

    // Vertex buffer
    buffer_destroy(context, &context->vertexIndexBuffer);
    TRACE("Vertex buffer destroyed");

    // Swapchain sync objects
    for(u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i){
    vkDestroySemaphore(context->device, context->swapchain.imageAvailableSemaphore[i], context->allocator);
    }

    for(u32 i = 0; i < context->swapchain.imageCount; ++i){    
    vkDestroySemaphore(context->device, context->swapchain.renderFinishedSemaphore[i], context->allocator);
    }

    for(u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i){
    vkDestroyFence(context->device, context->swapchain.inFlightFence[i], context->allocator);
    }

    darray_destroy(context->swapchain.imageAvailableSemaphore);
    darray_destroy(context->swapchain.renderFinishedSemaphore);
    darray_destroy(context->swapchain.inFlightFence);
    context->swapchain.imageAvailableSemaphore = 0;
    context->swapchain.renderFinishedSemaphore = 0;
    context->swapchain.inFlightFence = 0;

    // Graphics command pool
    command_buffers_cleanup(context, &context->graphics_command_buffer);
    TRACE("Graphics command pool and buffers destroyed");

    // Transfer command pool if any
    if(context->transferQueue){
    command_buffers_cleanup(context, &context->transfer_command_buffer);
    TRACE("Transfer command pool and buffers destroyed");
    }

    // Swapchain framebuffers
    for(u32 i = 0; i < context->swapchain.imageCount; ++i){
        vkDestroyFramebuffer(context->device, context->swapchain.swapchainFramebuffers[i], context->allocator);
    }
    context->swapchain.swapchainFramebuffers = 0;
    TRACE("Vulkan swapchain framebuffers destroyed");

    // Swapchain renderpass
    vkDestroyRenderPass(context->device, context->swapchain.renderpass, context->allocator);
    context->swapchain.renderpass = 0;
    TRACE("Vulkan swapchain renderpass destroyed");

    // Swapchain pipeline and layout
    vkDestroyPipeline(context->device, context->swapchain.graphicsPipeline, context->allocator);
    vkDestroyPipelineLayout(context->device, context->swapchain.pipelineLayout, context->allocator);
    context->swapchain.graphicsPipeline = 0;
    context->swapchain.pipelineLayout = 0;
    TRACE("Vulkan swapchain pipeline and layout destroyed");

    // Swapchain image views
    for(u32 i = 0; i < darray_capacity(context->swapchain.swapchainImageViews); ++i){
        vkDestroyImageView(context->device, context->swapchain.swapchainImageViews[i], context->allocator);
    }
    context->swapchain.swapchainImageViews = 0;
    TRACE("Vulkan swapchain image views destroyed");

    // Swapchain this goes before logical device?
    swapchain_cleanup(context, &context->swapchain);
    TRACE("Vulkan swapchain destroyed");

    // Uniform buffer objects
    for(u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i){
        vkDestroyBuffer(context->device, context->uniformBuffers[i].buffer, context->allocator);
        vkFreeMemory(context->device, context->uniformBuffers[i].bufferMemory, context->allocator);
        context->uniformBuffers[i].buffer = 0;
        context->uniformBuffers[i].bufferMemory = 0;
        context->uniformBuffers[i].map_ptr = 0;

    }
    TRACE("Destroyed uniform buffer objects")

    // Descriptor layout
    vkDestroyDescriptorSetLayout(context->device, context->ubo_descriptor.layout, context->allocator);
    context->ubo_descriptor.layout = 0;
    TRACE("Destroyed descriptor layouts");

    // Descriptor pool
    vkDestroyDescriptorPool(context->device, context->ubo_descriptor.pool, context->allocator);
    context->ubo_descriptor.sets = 0;
    TRACE("Descriptor pool destroyed");

    // Logical device
    vkDestroyDevice(context->device, context->allocator);
    context->device = 0;
    TRACE("Vulkan logical device destroyed");

    // Surface
    vkDestroySurfaceKHR(context->instance, context->surface, context->allocator);
    context->surface = 0;
    TRACE("Vulkan surface destroyed");

    // Debugger
    #ifdef _DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(context->instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func){
        func(context->instance, context->debugMessenger, context->allocator);
        context->debugMessenger = 0;
        TRACE("Vulkan debugger destroyed");
    } else {
        ERROR("Problem destroying debugger, extension not supported");
    }
    #endif

    // Instance
    vkDestroyInstance(context->instance, context->allocator);
    context->instance = 0;
    TRACE("Vulkan Instance destroyed");

    TRACE("Vulkan cleanup done");





    // GLFW cleanup
    glfwDestroyWindow(context->window);
    glfwTerminate();

    context->window = 0;

    TRACE("GLFW cleanup done");

    TRACE("App shutting down");
}






b8 initVulkan(vulkan_context* context){
    if(!createInstance(context)){
        ERROR("Instance creation failed!");
        return false;
    }

    SUCCESS("Instance created successfully!");

    // Surface creation
    if(!vulkan_result_is_success(glfwCreateWindowSurface(context->instance, context->window, context->allocator, &context->surface))){
        ERROR("Could not create surface");
        return false;
    }

    SUCCESS("Surface created successfully!");

    if(!pickPhysicalDevice(context)){
        ERROR("Could not find suitable physical device!");
        return false;
    }

    SUCCESS("Suitable device found!");

    if(!createLogicalDevice(context)){
        ERROR("Could not create logical device");
        return false;
    }

    SUCCESS("Created logical device!");

    // Getting queue handles

    // Graphics
    vkGetDeviceQueue(context->device, context->graphicsFamily, 0, &context->graphicsQueue);

    if(context->graphicsQueue){
    SUCCESS("Acquired graphics queue handle");
    } else {
        ERROR("Could not get graphics queue handle");
        return false;
    }

    // Present
    vkGetDeviceQueue(context->device, context->presentFamily, 0, &context->presentQueue);

    if(context->presentQueue){
    SUCCESS("Acquired present queue handle");
    } else {
        ERROR("Could not get present queue handle");
        return false;
    }

    // Transfer
    if(context->transferFamily != context->graphicsFamily || context->transferFamily != context->presentFamily || context->transferFamily != context->computeFamily){
    vkGetDeviceQueue(context->device, context->transferFamily, 0, &context->transferQueue);

    if(context->transferQueue){
    SUCCESS("Acquired transfer queue handle");
    } else {
        ERROR("Could not get transfer queue handle");
        return false;
    }
    }

    // Swapchain creation
    if(!swapchainCreate(context, &context->swapchain)){
        ERROR("Could not create swapchain");
        return false;
    }

    SUCCESS("Swapchain created successfully!");

    // Swapchain Image views
    if(!create_image_views(context, &context->swapchain.swapchainImageViews, context->swapchain.swapchainImages)){
        ERROR("Could not create image views");
        return false;
    }

    SUCCESS("Created swapchain image views!");

    // Swapchain Renderpass
    if(!create_renderpass(context, context->swapchain.format, &context->swapchain.renderpass)){
        ERROR("Could not create renderpass");
        return false;
    }

    SUCCESS("Created swapchain renderpass!");


    // Swapchain descriptors
    if(!create_descriptor_layout(context, &context->ubo_descriptor, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)){
        ERROR("Could not create swapchain descriptor layout");
        return false;
    }

    SUCCESS("Created swapchain descriptor layout");

    // Swapchain Pipeline
    // if we have multiple sets we need to specify multiple layouts here
    VkDescriptorSetLayout descriptor_layouts[] = {context->ubo_descriptor.layout};
    if(!create_graphics_pipeline(
        context,
        "assets/shaders/Builtin.MaterialShader.vert.spv",
        "assets/shaders/Builtin.MaterialShader.frag.spv",
        context->swapchain.extent.width,
        context->swapchain.extent.height,
        context->swapchain.extent,
        (sizeof(descriptor_layouts) / sizeof(VkDescriptorSetLayout)),
        descriptor_layouts,
        &context->swapchain.pipelineLayout,
        &context->swapchain.renderpass,
        &context->swapchain.graphicsPipeline
    )){
        ERROR("Could not create swapchain graphics pipeline")
        return false;
    }

    SUCCESS("Created swapchain graphics pipeline!");

    // Descriptor pool and sets
    if(!create_descriptor_pool(context, &context->ubo_descriptor, MAX_FRAMES_IN_FLIGHT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)){
        ERROR("Could not create ubo descriptor pool");
        return false;
    }

    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {
        context->ubo_descriptor.layout,
        context->ubo_descriptor.layout
    };
    context->ubo_descriptor.sets = darray_reserve(VkDescriptorSet, MAX_FRAMES_IN_FLIGHT);
    if(!create_descriptor_sets(context, &context->ubo_descriptor, MAX_FRAMES_IN_FLIGHT, layouts)){
        ERROR("Could not create ubo descriptor sets");
        return false;
    }

    SUCCESS("Created ubo descriptor pool and sets");

    // Swapchain framebuffers
    if(!create_frambuffers(
        context,
        context->swapchain.imageCount,
        context->swapchain.extent.width,
        context->swapchain.extent.height,
        &context->swapchain.swapchainFramebuffers,
        context->swapchain.swapchainImageViews,
        &context->swapchain.renderpass
    )){
        ERROR("Could not create swapchain framebuffers");
        return false;
    }

    SUCCESS("Created swapchain framebuffers!");

    // Graphics queue command pool
    if(!create_command_buffer_pool(context, context->graphicsFamily, &context->graphics_command_buffer)){
        ERROR("Could not create graphics command pool");
        return false;
    }

    SUCCESS("Created graphics queue command pool");

    // Graphics command pool command buffers
    context->graphics_command_buffer.commandbuffers = darray_reserve(VkCommandBuffer, MAX_FRAMES_IN_FLIGHT);
    if(!create_command_buffers(context, context->graphics_command_buffer.commandPool, context->graphics_command_buffer.commandbuffers, MAX_FRAMES_IN_FLIGHT)){
        ERROR("Could not create graphics command pool command buffers");
        return false;
    }

    // Transfer queue command pool if any

    if(context->transferQueue){
    if(!create_command_buffer_pool(context, context->transferFamily, &context->transfer_command_buffer)){
        ERROR("Could not create transfer command pool");
        return false;
    }

    SUCCESS("Created transfer queue command pool");

    // Transfer command pool command buffers
    context->transfer_command_buffer.commandbuffers = darray_reserve(VkCommandBuffer, MAX_FRAMES_IN_FLIGHT);
    if(!create_command_buffers(context, context->transfer_command_buffer.commandPool, context->transfer_command_buffer.commandbuffers, MAX_FRAMES_IN_FLIGHT)){
        ERROR("Could not create transfer command pool command buffers");
        return false;
    }

    SUCCESS("Created transfer queue command pool command buffers");
    }

    // Swapchain semaphores and fences
    context->swapchain.imageAvailableSemaphore = darray_reserve(VkSemaphore, MAX_FRAMES_IN_FLIGHT);
    context->swapchain.renderFinishedSemaphore = darray_reserve(VkSemaphore, context->swapchain.imageCount); // depending on images I need this many semaphores for the swapchain to work!
    context->swapchain.inFlightFence = darray_reserve(VkFence, MAX_FRAMES_IN_FLIGHT);

    if(!create_semaphores(context, MAX_FRAMES_IN_FLIGHT, context->swapchain.imageAvailableSemaphore)){
        ERROR("Could not create image available semaphores");
        return false;
    }

    if(!create_semaphores(context, context->swapchain.imageCount, context->swapchain.renderFinishedSemaphore)){
        ERROR("Could not create render finished semaphores");
        return false;
    }

    if(!create_fences(context, MAX_FRAMES_IN_FLIGHT, context->swapchain.inFlightFence)){
        ERROR("Could not create in flight fences");
        return false;
    }

    SUCCESS("Created swapchain sync objects");

    // Vertex buffer sharing if transfer and graphics are different
    VkSharingMode vertexIndexBufferSharing = VK_SHARING_MODE_EXCLUSIVE;
    u32 family_indices[] = {
        context->graphicsFamily,
        context->transferFamily
    };

    if(context->transferFamily != context->graphicsFamily) {
        vertexIndexBufferSharing = VK_SHARING_MODE_CONCURRENT;
    }

    
    // TODO: Hardcoded vertices and indices
    vertex_3d vertices[] = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    // BEWARE CULLING ORDER!!
    // THESE ARE CW BECAUSE WE FLIP THE Y-AXIS TO CONFORM WITH VULKAN'S NDC COORDINATES IN THE DRAWFRAME FUNC
    // WE ARE USING A RIGHT HAND COORDINATE SYSTEM
    u16 indices[] = {
        0, 1, 2, 2, 3, 0
    };
    // end TODO:

    // Vertex buffers
    #define VERTEX_BUFFER_SIZE 1024 // I can do this right? no retard
    #define INDEX_BUFFER_SIZE 128
    if(!create_buffer(context, 
        &context->vertexIndexBuffer, 
        (sizeof(vertices) + sizeof(indices)),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        vertexIndexBufferSharing,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        family_indices, 2)){
        
        ERROR("Could not create vertex index buffer");
        return false;
    }

    context->vertexIndexBuffer.offsets[0] = sizeof(vertices);
    SUCCESS("Vertex index buffer created");

    // Staging vertex buffer
    if(!create_buffer(context, 
        &context->vertexIndexBufferStaging, 
        (sizeof(vertices) + sizeof(indices)),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        0, 0)){
        
        ERROR("Could not create staging vertex index buffer");
        return false;
    }

    SUCCESS("Staging vertex index buffer created");

    context->vertexIndexBufferStaging.offsets[0] = context->vertexIndexBuffer.offsets[0];

    map_memory(context, context->vertexIndexBufferStaging.bufferMemory, sizeof(vertices), 0, vertices);
    map_memory(context, context->vertexIndexBufferStaging.bufferMemory, sizeof(indices), (u32)context->vertexIndexBufferStaging.offsets[0], indices);


    // Buffer copy
    VkCommandBuffer commandbuffer;
    if(context->transfer_command_buffer.commandbuffers){
    commandbuffer = context->transfer_command_buffer.commandbuffers[0];
    } else {
        create_command_buffers(context, context->graphics_command_buffer.commandPool, &commandbuffer, 1);
    }

    // TODO: turn all this into a helper func
    record_command_buffer_one_time(commandbuffer);
    copy_buffer(context, context->vertexIndexBufferStaging.buffer, context->vertexIndexBuffer.buffer, context->vertexIndexBufferStaging.size, commandbuffer);
    end_command_buffer(commandbuffer);
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandbuffer;

    if(context->transferQueue){
    vkQueueSubmit(context->transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(context->transferQueue);
    reset_command_buffer(commandbuffer);
    } else {
    vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(context->graphicsQueue);
    free_command_buffers(context, context->graphics_command_buffer.commandPool, 1, &commandbuffer);
    }

    buffer_destroy(context, &context->vertexIndexBufferStaging);

    SUCCESS("Copied staging vertex index buffer contents");

    // Uniform buffer
    for(u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i){

    if(!create_buffer(context, &context->uniformBuffers[i], sizeof(ubo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0, 0)){
        ERROR("Failed to create uniform_buffer[%i]", i);
        return false;
    }

    // Persistent mapping
    vkMapMemory(context->device, context->uniformBuffers[i].bufferMemory, 0, context->uniformBuffers[i].size, 0, &context->uniformBuffers[i].map_ptr);

    }

    SUCCESS("Created uniform buffers");

    return true;
}


// Helper funcs

b8 createInstance(vulkan_context* context){

    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pApplicationName = "Vulkan Game";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;
    
    // Extensions
    u32 glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    const char** req_extensions = darray_create(const char*);
    for(u32 i = 0; i < glfwExtensionCount; ++i){
        darray_push(req_extensions, glfwExtensions[i]);
    }
   
    #ifdef _DEBUG
    glfwExtensionCount++;
    darray_push(req_extensions, (const char*)"VK_EXT_debug_utils"); // string literals are interpreted as char arrays :| wtvr
    #endif

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = req_extensions;

    u32 extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0);
    VkExtensionProperties* extensions = darray_reserve(VkExtensionProperties, extensionCount);
    vkEnumerateInstanceExtensionProperties(0, &extensionCount, extensions);

    for(u32 i = 0; i < glfwExtensionCount; ++i){
        LOG("Needed extension: %s", req_extensions[i]);
        b8 found = false;
        for(u32 j = 0; j < extensionCount; ++j){
            if(strcmp(req_extensions[i], extensions[j].extensionName) == 0){
                found = true;
                SUCCESS("Extension found!");
                break;
            }
        }

        if(!found){
            ERROR("Extension couldn't be found!");
            return false;
        }
    }

    // Validation layers
    createInfo.enabledLayerCount = 0;

    #ifdef _DEBUG

    #define LAYER_COUNT 1
    const char* validationLayers[LAYER_COUNT] = {
    "VK_LAYER_KHRONOS_validation"
    };
    

    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, 0);
    VkLayerProperties* availableLayers = darray_reserve(VkLayerProperties, layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
    
    for(u32 i = 0; i < LAYER_COUNT; ++i){
        SUCCESS("Needed layer: %s", validationLayers[i]);
        b8 layer_found = false;
        for(u32 j = 0; j < layerCount; ++j){
            if(strcmp(validationLayers[i], availableLayers[j].layerName) == 0){
                layer_found = true;
                SUCCESS("Layer found!");
                break;
            }
        }

        if(!layer_found){
            ERROR("Layer couldn't be found!");
            return false;
        }
    }

    createInfo.enabledLayerCount = LAYER_COUNT;
    createInfo.ppEnabledLayerNames = validationLayers;

    // Debugger
    VkDebugUtilsMessengerCreateInfoEXT debug_createInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    debug_createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_createInfo.pfnUserCallback = debugCallback;
    debug_createInfo.pUserData = 0;
    #endif

    VK_CHECK(vkCreateInstance(&createInfo, context->allocator, &context->instance);

    #ifdef _DEBUG

        PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(context->instance, "vkCreateDebugUtilsMessengerEXT");
    if(func){
        VK_CHECK(func(context->instance, &debug_createInfo, context->allocator, &context->debugMessenger));
    } else {
        ERROR("Debug extension not supported!");
        return false;
    }

    SUCCESS("Vulkan debugger created!");

    #endif

    darray_destroy(availableLayers);
    darray_destroy(extensions);
    darray_destroy(req_extensions);


    return true;
)
}

b8 pickPhysicalDevice(vulkan_context* context){
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(context->instance, &deviceCount, 0);

    if(deviceCount == 0){
        ERROR("No Vulkan supported devices found!");
        return false;
    }

    VkPhysicalDevice* devices = darray_reserve(VkPhysicalDevice, deviceCount);

    vkEnumeratePhysicalDevices(context->instance, &deviceCount, devices);

    for(u32 i = 0; i < deviceCount; ++i){
        if(isDeviceSuitable(devices[i])){
            context->physicalDevice = devices[i];
            break;
        }
    }

    if(context->physicalDevice == VK_NULL_HANDLE){
        return false;
    }

    // Queue families
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(context->physicalDevice, &queueFamilyCount, 0);
    VkQueueFamilyProperties* queueFamilies = darray_reserve(VkQueueFamilyProperties, queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(context->physicalDevice, &queueFamilyCount, queueFamilies);

    
    u32 min_pqueues = UINT32_MAX;
    u32 min_gqueues = UINT32_MAX;
    u32 min_tqueues = UINT32_MAX;
    u32 min_cqueues = UINT32_MAX;
    for(u32 i = 0; i < queueFamilyCount; ++i){
        
        // Graphics
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            if(queueFamilies[i].queueCount < min_gqueues){
                min_gqueues = queueFamilies[i].queueCount;
                context->graphicsFamily = i;
            }
        }


        // Compute
        if(queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT){
            if(queueFamilies[i].queueCount < min_cqueues){
                min_cqueues = queueFamilies[i].queueCount;
                context->computeFamily = i;
            }
        }


        // Transfer
        if(queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT){
            if(queueFamilies[i].queueCount < min_tqueues){
                min_tqueues = queueFamilies[i].queueCount;
                context->transferFamily = i;
            }
        }



        // Present
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(context->physicalDevice, i, context->surface, &presentSupport);
        if(presentSupport){
            if(queueFamilies[i].queueCount < min_pqueues){
                min_pqueues = queueFamilies[i].queueCount;
                context->presentFamily = i;
            }
        }
        
    }

    VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(context->physicalDevice, context->graphicsFamily, context->surface, &presentSupport);
        if(presentSupport){
                context->presentFamily = context->graphicsFamily;
        }

     // Hack hack hack hack
    if(!querySwapchainSupport(context, &context->swapchain)){
        ERROR("Could not query for swapchain support");
        return false;
    }

    darray_destroy(queueFamilies);
    darray_destroy(devices);

     // Properties
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(context->physicalDevice, &deviceProperties);

    LOG("Device: %s", deviceProperties.deviceName);
    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
        LOG("Type: %s", "Discrete GPU");
    } else if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
        LOG("Type: %s", "Integrated GPU");
    } else if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU){
        LOG("Type: %s", "CPU");
    } else {
        LOG("Type: Unknown");
    }

    LOG("Graphics queue family: %i\nCompute queue family: %i\nTransfer queue family: %i\nPresent queue family: %i", context->graphicsFamily, context->computeFamily, context->transferFamily, context->presentFamily);

    return true;

}

b8 isDeviceSuitable(VkPhysicalDevice device){

    // Properties
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);


    // Features
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // ignore this for now
    if(!deviceFeatures.geometryShader || !deviceFeatures.tessellationShader) {
        ERROR("Physical device is not discrete, or does not support geometry/tessellation shaders");
        return false;
    }
    

    // anisotropic filtering
    if(!deviceFeatures.samplerAnisotropy){
        ERROR("Physical device does not support anisotropic filtering!");
        return false;
    }

    // Check extensions
    // HACK: Oops, I forgot the context is defined in this file statically, hack, whatever :-)
    context.requiredExtensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    // Get extensions
    u32 extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, 0, &extensionCount, 0); // layer??
    VkExtensionProperties* availableExtensions = darray_reserve(VkExtensionProperties, extensionCount);
    vkEnumerateDeviceExtensionProperties(device, 0, &extensionCount, availableExtensions); 

    for(u32 i = 0; i < EXTENSION_COUNT; ++i){

        b8 found = false;
        SUCCESS("Needed device extension: %s", context.requiredExtensions[i]);

        for(u32 j = 0; j < extensionCount; ++j){        
        if(strcmp(context.requiredExtensions[i], availableExtensions[j].extensionName) == 0){
        
            SUCCESS("Extension found!");
            found = true;
            break;
        }
        }

        if(!found){
            ERROR("Extension not found");
            return false;
        }
    }

    darray_destroy(availableExtensions);
    
    return true;
}

b8 createLogicalDevice(vulkan_context* context){

    // Creating graphics (and/or present queue) queue
    VkDeviceQueueCreateInfo* queueInfos = darray_create(VkDeviceQueueCreateInfo);
    u32 queueInfosCount = 1;

    VkDeviceQueueCreateInfo queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueCreateInfo.queueFamilyIndex = context->graphicsFamily;
    queueCreateInfo.queueCount = 1; // only one queue, so you should submit all commands at the same time

    f32 queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    darray_push(queueInfos, queueCreateInfo);

    if(context->graphicsFamily != context->presentFamily){
    queueInfosCount++;
        
    VkDeviceQueueCreateInfo pqueueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    pqueueCreateInfo.queueFamilyIndex = context->presentFamily;
    pqueueCreateInfo.queueCount = 1; // only one queue, so you should submit all commands at the same time

    pqueueCreateInfo.pQueuePriorities = &queuePriority;

    darray_push(queueInfos, pqueueCreateInfo);
    }

    if(context->transferFamily != context->graphicsFamily || context->transferFamily != context->presentFamily || context->transferFamily != context->computeFamily){
    queueInfosCount++;
        
    VkDeviceQueueCreateInfo tqueueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    tqueueCreateInfo.queueFamilyIndex = context->transferFamily;
    tqueueCreateInfo.queueCount = 1; // only one queue, so you should submit all commands at the same time

    tqueueCreateInfo.pQueuePriorities = &queuePriority;

    darray_push(queueInfos, tqueueCreateInfo);
    }


    // Device Features
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(context->physicalDevice, &deviceFeatures);

    // Logical device creation
    VkDeviceCreateInfo createInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    createInfo.pQueueCreateInfos = queueInfos;
    createInfo.queueCreateInfoCount = queueInfosCount;
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = EXTENSION_COUNT;
    createInfo.ppEnabledExtensionNames = context->requiredExtensions;

    createInfo.enabledLayerCount = 0; // for validation layers, when instance and device layers were different, now is no longer the case and these are ignored

    VK_CHECK(vkCreateDevice(context->physicalDevice, &createInfo, context->allocator, &context->device));

    darray_destroy(queueInfos);

    return true;
}


b8 resize_swapchain(vulkan_context* context, vulkan_swapchain* swapchain){

    // Checks
    i32 width = 0, height = 0;
    glfwGetFramebufferSize(context->window, &width, &height);

    if(context->height == height && context->width == width){
        return true; // early return in case the resize doesn't change anything?
    }

    while (width == 0 || height == 0){
        TRACE("Window minimized");
        glfwWaitEvents();
        glfwGetFramebufferSize(context->window, &width, &height); // shouldn't it be after the wait?
    }

    vkDeviceWaitIdle(context->device);

    // syncing
    context->width = width;
    context->height = height;

    // cleanup
    // Swapchain framebuffers
    for(u32 i = 0; i < swapchain->imageCount; ++i){
        vkDestroyFramebuffer(context->device, swapchain->swapchainFramebuffers[i], context->allocator);
    }
    //TRACE("Vulkan swapchain framebuffers destroyed");

    // Swapchain renderpass
    vkDestroyRenderPass(context->device, swapchain->renderpass, context->allocator);
   // TRACE("Vulkan swapchain renderpass destroyed");

    // Swapchain pipeline and layout (we dont do pipeline becasuse we set viewport and scissor to be dynamic?)
    // vkDestroyPipeline(context->device, context->swapchain.graphicsPipeline, context->allocator);
    //vkDestroyPipelineLayout(context->device, context->swapchain.pipelineLayout, context->allocator);
    // TRACE("Vulkan swapchain pipeline and layout destroyed");

    // Swapchain image views
    for(u32 i = 0; i < swapchain->imageCount; ++i){
        vkDestroyImageView(context->device, swapchain->swapchainImageViews[i], context->allocator);
    }
   // TRACE("Vulkan swapchain image views destroyed");

    // Swapchain this goes before logical device?
    swapchain_cleanup(context, swapchain);
   // TRACE("Vulkan swapchain destroyed");





    // recreate
    if(!querySwapchainSupport(context, swapchain)){
        ERROR("Could not query for swapchain support");
        return false;
    }

    // Swapchain creation
    if(!swapchainCreate(context, swapchain)){
        ERROR("Could not create swapchain");
        return false;
    }

    //SUCCESS("Swapchain created successfully!");

    // Swapchain Image views
    if(!create_image_views(context, &swapchain->swapchainImageViews, context->swapchain.swapchainImages)){
        ERROR("Could not create image views");
        return false;
    }

   // SUCCESS("Created swapchain image views!");

    // Swapchain Renderpass
    if(!create_renderpass(context, context->swapchain.format, &swapchain->renderpass)){
        ERROR("Could not create renderpass");
        return false;
    }

    //SUCCESS("Created swapchain renderpass!");

    // Swapchain Pipeline
    /*if(!create_graphics_pipeline(
        context,
        "assets/shaders/Builtin.MaterialShader.vert.spv",
        "assets/shaders/Builtin.MaterialShader.frag.spv",
        swapchain->extent.width,
        swapchain->extent.height,
        swapchain->extent,
        &swapchain->pipelineLayout,
        &swapchain->renderpass,
        &swapchain->graphicsPipeline
    )){
        ERROR("Could not create swapchain graphics pipeline")
        return false;
    }

    SUCCESS("Created swapchain graphics pipeline!");
    */

    // Swapchain framebuffers
    if(!create_frambuffers(
        context,
        swapchain->imageCount,
        swapchain->extent.width,
        swapchain->extent.height,
        &swapchain->swapchainFramebuffers,
        swapchain->swapchainImageViews,
        &swapchain->renderpass
    )){
        ERROR("Could not create swapchain framebuffers");
        return false;
    }

    //SUCCESS("Created swapchain framebuffers!");

    TRACE("New window size: %iw %ih", context->width, context->height);

    return true;
}
