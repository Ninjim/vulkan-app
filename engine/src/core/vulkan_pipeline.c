#include "vulkan_pipeline.h"
#include "vulkan_shader_module.h"
#include "math/math_types.h"

b8 create_graphics_pipeline(vulkan_context* context, const char* vertPath, const char* fragPath, u32 width, u32 height, VkExtent2D extent, u32 layout_count, VkDescriptorSetLayout* descriptor_layous, VkPipelineLayout* pipelineLayout, VkRenderPass* renderpass, VkPipeline* pipeline){
    
    // There are copies taken by the pipeline I think of the shader modules
    // so after we create the pipeline, we can delete them
    VkShaderModule vertShaderModule, fragShaderModule;
    create_shader_module(context, vertPath, &vertShaderModule);
    create_shader_module(context, fragPath, &fragShaderModule);

    // vert shader stage
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = 0; // in case of constants in the shader, we can specify them here so Vulkan can optimize the shader execution?

    // frag shader stage
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = 0; // in case of constants in the shader, we can specify them here so Vulkan can optimize the shader execution?

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // DYNAMIC STATE IF ANY
    #define DYNAMIC_STATES 2
    VkDynamicState dynamicStates[DYNAMIC_STATES] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicStateInfo.dynamicStateCount = DYNAMIC_STATES;
    dynamicStateInfo.pDynamicStates = dynamicStates;
    

    // Vertex input
    // Binding descriptions
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0; // for vertex buffer?? and also attribute description
    bindingDescription.stride = sizeof(vertex_3d);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Attribute descriptions
    #define VERTEX_MEMBERS 2
    vertex_3d example = {};
    VkVertexInputAttributeDescription attributeDescriptions[VERTEX_MEMBERS] = {};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = 0;
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = sizeof(example.position);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions; // this is actually for where the data will go inside the shader
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // bindings is spacing between data, and instancing is when we tell the driver not to load data per-vertex, but per-instance (or I guess per-"mesh" would be clearer to say), like with trees for example
    vertexInputInfo.vertexAttributeDescriptionCount = VERTEX_MEMBERS;

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE; // if when using strip options, we want the ability to break apart groups with a special index

    // Viewport and scissor  (USING DYNAMIC STATE NOT USED NOW)
    VkViewport viewport = {}; // squashes the image into this resolution
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)width;
    viewport.height = (f32)height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f; // reverse values for reverse depth testing or?
    
    VkRect2D scissor = {}; // actually cuts off parts outside of this range
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    viewportState.pScissors = 0; // these two are not defined now because the states are dynamic, they are configured at draw time
    viewportState.pViewports = 0;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizer.depthClampEnable = VK_FALSE; // if fragments beyond the near and far planes should be clamped inside, if so this needs a GPU feature to work
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // for offscreen rendering basically, saying that the rasterizer shouldn't happen, therefore fragmetns don't happen and you can't output to framebuffers
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // can enable wireframe! if GPU allows it, also feature
    rasterizer.lineWidth = 1.0f; // anything thicker than 1.0f is a GPU feature (wideLines)
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // back-face culling shefa e pisal ccw znachi i az pisha slusham i izpalnqvam
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f; // these are for shadow maps!

    // TODO: Multisampling requires GPU feature
    VkPipelineMultisampleStateCreateInfo multisampling = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = 0;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // TODO: Depth and stencil testing
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment; // per framebuffer
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // which channels we care about
    // the following uses color blending based on opacity, which I feel like could work on like, glass and stuff no? but if it's a separate framebuffer i guess right :( idk, in any case the alpha here stays max so it shouldn't matter
    // but no this works on old and new framebuffer data, and we don't keep the old, so this doesn't do anything
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // still ignoring new alpha value
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlending.logicOpEnable = VK_FALSE; // if we want to use bitwise operations isntead of the claculations in the previous struct, basically setting blendEnable to false
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // idk what they do why theyre here and why there are 4
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // TODO: Pipeline layout (for uniforms)
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipelineLayoutInfo.setLayoutCount = layout_count;
    pipelineLayoutInfo.pSetLayouts = descriptor_layous;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = 0;

    // Create pipeline layout
    VK_CHECK(vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, context->allocator, pipelineLayout));

    // Creating pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = 0; // optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = *pipelineLayout;
    pipelineInfo.renderPass = *renderpass; // can use other renderpasses, doesn't have to be this one, but they have to be compatible with this one
    pipelineInfo.subpass = 0; // index of subpass where this pipeline will be used, so the bind point I guess?
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // create a pipeline from previous one, can only be used if VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is set
    pipelineInfo.basePipelineIndex = -1; // idk where u get this index from even??

    // Pipeline cache very useful!
    VK_CHECK(vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo, context->allocator, pipeline));

    // cleanup
    vkDestroyShaderModule(context->device, fragShaderModule, context->allocator);
    vkDestroyShaderModule(context->device, vertShaderModule, context->allocator);


    return true;
}






b8 pipeline_draw(VkCommandBuffer commandbuffer, VkPipeline graphics_pipeline, VkPipelineLayout layout, u32 width, u32 height, VkExtent2D extent, VkDescriptorSet* descriptorsets, u32 descriptorset_count, u64 v_count, u64 i_count, u64 v_offset, u64 i_offset, VkBuffer* vertexBuffers, VkDeviceSize* vertexOffsets, VkDeviceSize indexOffset, u32 indices, u32 index_instances, u32 indices_offset,u32 indices_vertex_offsets, u32 indance_instance_offset){

    vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    if(width != 0 && height != 0){
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = width;
        viewport.height = height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandbuffer, 0, 1, &viewport);
    }

    if(extent.height != 0 && extent.width != 0){
        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent = extent;
        vkCmdSetScissor(commandbuffer, 0, 1, &scissor);
    }

    // Bind vertex buffers
    vkCmdBindVertexBuffers(commandbuffer, 0, 1, vertexBuffers, vertexOffsets);
    vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, descriptorset_count, descriptorsets, 0, 0);

    if(indices != 0){
        vkCmdBindIndexBuffer(commandbuffer, vertexBuffers[0], indexOffset, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(commandbuffer, indices, index_instances, indices_offset, indices_vertex_offsets, indance_instance_offset);
    }
    else{
    vkCmdDraw(commandbuffer, v_count, i_count, v_offset, i_offset);
    }

    return true;
}