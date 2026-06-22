/*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#include <awn.hpp>

#define DEFINE_VK_PROC(name) PFN_##name pfn_##name

namespace {

    void ProcAbort([[maybe_unused]] const char *name) {
        VP_ASSERT(false);
    }
}

#ifdef VP_TARGET_PLATFORM_win32
    #define LOAD_INITIAL_VK_PROC(name) pfn_##name = reinterpret_cast<PFN_##name>(::GetProcAddress(sVulkanDLL, #name)); if (pfn_##name == nullptr) { ProcAbort(#name); };

    HMODULE sVulkanDLL = nullptr;
    void LoadVulkanLibrary() {
        sVulkanDLL = ::LoadLibraryA("vulkan-1.dll");
        VP_ASSERT(sVulkanDLL != nullptr);
    }
    
#elifdef VP_TARGET_PLATFORM_linux
    #define LOAD_INITIAL_VK_PROC(name) pfn_##name = reinterpret_cast<PFN_##name>(::dlsym(sVulkanDLL, #name)); if (pfn_##name == nullptr) { ProcAbort(#name); };

    void *sVulkanDLL = nullptr;
    void LoadVulkanLibrary() {
        sVulkanDLL = ::dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
        VP_ASSERT(sVulkanDLL != nullptr);
    }
#endif

#define LOAD_INSTANCE_VK_PROC(name) pfn_##name = reinterpret_cast<PFN_##name>(::pfn_vkGetInstanceProcAddr(instance, #name)); if (pfn_##name == nullptr) { ProcAbort(#name); };
#define LOAD_DEVICE_VK_PROC(name)   pfn_##name = reinterpret_cast<PFN_##name>(::pfn_vkGetDeviceProcAddr(device, #name)); if (pfn_##name == nullptr) { ProcAbort(#name); };

/* Initial procs */
DEFINE_VK_PROC(vkCreateInstance);
DEFINE_VK_PROC(vkEnumerateInstanceExtensionProperties);
DEFINE_VK_PROC(vkEnumerateInstanceLayerProperties);
DEFINE_VK_PROC(vkGetInstanceProcAddr);

/* Initial Vulkan 1.1 */
DEFINE_VK_PROC(vkEnumerateInstanceVersion);

#pragma GCC diagnostic ignored "-Wcast-function-type"

void LoadVkCProcsInitial() {

    /* Load vulkan dll */
    LoadVulkanLibrary();

    /* Initial procs */
    LOAD_INITIAL_VK_PROC(vkCreateInstance);
    LOAD_INITIAL_VK_PROC(vkEnumerateInstanceExtensionProperties);
    LOAD_INITIAL_VK_PROC(vkEnumerateInstanceLayerProperties);
    LOAD_INITIAL_VK_PROC(vkGetInstanceProcAddr);

    /* Initial Vulkan 1.1 */
    LOAD_INITIAL_VK_PROC(vkEnumerateInstanceVersion);

    return;
}

/* Instance procs */
DEFINE_VK_PROC(vkCreateDevice);
DEFINE_VK_PROC(vkDestroyDevice);
DEFINE_VK_PROC(vkDestroyInstance);
DEFINE_VK_PROC(vkEnumerateDeviceExtensionProperties);
DEFINE_VK_PROC(vkEnumerateDeviceLayerProperties);
DEFINE_VK_PROC(vkEnumeratePhysicalDeviceGroups);
DEFINE_VK_PROC(vkEnumeratePhysicalDevices);
DEFINE_VK_PROC(vkGetDeviceProcAddr);
DEFINE_VK_PROC(vkGetPhysicalDeviceFeatures);
DEFINE_VK_PROC(vkGetPhysicalDeviceFormatProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceImageFormatProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceMemoryProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceQueueFamilyProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceSparseImageFormatProperties);

/* Instance Vulkan 1.1 */
DEFINE_VK_PROC(vkGetPhysicalDeviceExternalBufferProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceExternalFenceProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceExternalSemaphoreProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceFeatures2);
DEFINE_VK_PROC(vkGetPhysicalDeviceFormatProperties2);
DEFINE_VK_PROC(vkGetPhysicalDeviceImageFormatProperties2);
DEFINE_VK_PROC(vkGetPhysicalDeviceMemoryProperties2);
DEFINE_VK_PROC(vkGetPhysicalDeviceProperties2);
DEFINE_VK_PROC(vkGetPhysicalDeviceQueueFamilyProperties2);
DEFINE_VK_PROC(vkGetPhysicalDeviceSparseImageFormatProperties2);

/* Instance Vulkan 1.3 */
DEFINE_VK_PROC(vkGetPhysicalDeviceToolProperties);

/* Instance VK_KHR_surface */
DEFINE_VK_PROC(vkDestroySurfaceKHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);

#ifdef VP_TARGET_PLATFORM_win32
/* Instance VK_KHR_win32_surface */
DEFINE_VK_PROC(vkCreateWin32SurfaceKHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceWin32PresentationSupportKHR);
#endif

/* Instance VK_KHR_cooperative_matrix */
DEFINE_VK_PROC(vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR);

/* Instance VK_KHR_swapchain */
DEFINE_VK_PROC(vkGetPhysicalDevicePresentRectanglesKHR);

/* Instance VK_KHR_get_surface_capabilities2 */
DEFINE_VK_PROC(vkGetPhysicalDeviceSurfaceCapabilities2KHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceSurfaceFormats2KHR);

/* Instance VK_EXT_debug_utils */
DEFINE_VK_PROC(vkCreateDebugUtilsMessengerEXT);
DEFINE_VK_PROC(vkDestroyDebugUtilsMessengerEXT);

void LoadVkCProcsInstance(VkInstance instance) {

    /* Instance procs */
    LOAD_INSTANCE_VK_PROC(vkCreateDevice);
    LOAD_INSTANCE_VK_PROC(vkDestroyDevice);
    LOAD_INSTANCE_VK_PROC(vkDestroyInstance);
    LOAD_INSTANCE_VK_PROC(vkEnumerateDeviceExtensionProperties);
    LOAD_INSTANCE_VK_PROC(vkEnumerateDeviceLayerProperties);
    LOAD_INSTANCE_VK_PROC(vkEnumeratePhysicalDeviceGroups);
    LOAD_INSTANCE_VK_PROC(vkEnumeratePhysicalDevices);
    LOAD_INSTANCE_VK_PROC(vkGetDeviceProcAddr);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceFeatures);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceFormatProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceImageFormatProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceMemoryProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceQueueFamilyProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSparseImageFormatProperties);

    /* Instance Vulkan 1.1 */
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceExternalBufferProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceExternalFenceProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceExternalSemaphoreProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceFeatures2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceFormatProperties2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceImageFormatProperties2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceMemoryProperties2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceProperties2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceQueueFamilyProperties2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSparseImageFormatProperties2);

    /* Instance Vulkan 1.3 */
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceToolProperties);

    /* Instance VK_KHR_surface */
    LOAD_INSTANCE_VK_PROC(vkDestroySurfaceKHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);

#ifdef VP_TARGET_PLATFORM_win32
    /* Instance VK_KHR_win32_surface */
    LOAD_INSTANCE_VK_PROC(vkCreateWin32SurfaceKHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceWin32PresentationSupportKHR);
#endif

    /* Instance VK_KHR_cooperative_matrix */
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR);

    /* Instance VK_KHR_swapchain */
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDevicePresentRectanglesKHR);

    /* Instance VK_KHR_get_surface_capabilities2 */
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSurfaceCapabilities2KHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSurfaceFormats2KHR);

    /* Instance VK_EXT_debug_utils */
#ifdef AWN_VK_DEBUG
    LOAD_INSTANCE_VK_PROC(vkCreateDebugUtilsMessengerEXT);
    LOAD_INSTANCE_VK_PROC(vkDestroyDebugUtilsMessengerEXT);
#endif

    return;
}

/* Device procs */
DEFINE_VK_PROC(vkAllocateCommandBuffers);
DEFINE_VK_PROC(vkAllocateDescriptorSets);
DEFINE_VK_PROC(vkAllocateMemory);
DEFINE_VK_PROC(vkBeginCommandBuffer);
DEFINE_VK_PROC(vkBindBufferMemory);
DEFINE_VK_PROC(vkBindImageMemory);
DEFINE_VK_PROC(vkCmdBeginQuery);
DEFINE_VK_PROC(vkCmdBeginRenderPass);
DEFINE_VK_PROC(vkCmdBindDescriptorSets);
DEFINE_VK_PROC(vkCmdBindIndexBuffer);
DEFINE_VK_PROC(vkCmdBindPipeline);
DEFINE_VK_PROC(vkCmdBindVertexBuffers);
DEFINE_VK_PROC(vkCmdBlitImage);
DEFINE_VK_PROC(vkCmdClearAttachments);
DEFINE_VK_PROC(vkCmdClearColorImage);
DEFINE_VK_PROC(vkCmdClearDepthStencilImage);
DEFINE_VK_PROC(vkCmdCopyBuffer);
DEFINE_VK_PROC(vkCmdCopyBufferToImage);
DEFINE_VK_PROC(vkCmdCopyImage);
DEFINE_VK_PROC(vkCmdCopyImageToBuffer);
DEFINE_VK_PROC(vkCmdCopyQueryPoolResults);
DEFINE_VK_PROC(vkCmdDispatch);
DEFINE_VK_PROC(vkCmdDispatchIndirect);
DEFINE_VK_PROC(vkCmdDraw);
DEFINE_VK_PROC(vkCmdDrawIndexed);
DEFINE_VK_PROC(vkCmdDrawIndexedIndirect);
DEFINE_VK_PROC(vkCmdDrawIndirect);
DEFINE_VK_PROC(vkCmdEndQuery);
DEFINE_VK_PROC(vkCmdEndRenderPass);
DEFINE_VK_PROC(vkCmdExecuteCommands);
DEFINE_VK_PROC(vkCmdFillBuffer);
DEFINE_VK_PROC(vkCmdNextSubpass);
DEFINE_VK_PROC(vkCmdPipelineBarrier);
DEFINE_VK_PROC(vkCmdPushConstants);
DEFINE_VK_PROC(vkCmdResetEvent);
DEFINE_VK_PROC(vkCmdResetQueryPool);
DEFINE_VK_PROC(vkCmdResolveImage);
DEFINE_VK_PROC(vkCmdSetBlendConstants);
DEFINE_VK_PROC(vkCmdSetDepthBounds);
DEFINE_VK_PROC(vkCmdSetEvent);
DEFINE_VK_PROC(vkCmdSetLineWidth);
DEFINE_VK_PROC(vkCmdSetScissor);
DEFINE_VK_PROC(vkCmdSetStencilCompareMask);
DEFINE_VK_PROC(vkCmdSetStencilReference);
DEFINE_VK_PROC(vkCmdSetStencilWriteMask);
DEFINE_VK_PROC(vkCmdSetViewport);
DEFINE_VK_PROC(vkCmdUpdateBuffer);
DEFINE_VK_PROC(vkCmdWaitEvents);
DEFINE_VK_PROC(vkCmdWriteTimestamp);
DEFINE_VK_PROC(vkCreateBuffer);
DEFINE_VK_PROC(vkCreateBufferView);
DEFINE_VK_PROC(vkCreateCommandPool);
DEFINE_VK_PROC(vkCreateComputePipelines);
DEFINE_VK_PROC(vkCreateDescriptorPool);
DEFINE_VK_PROC(vkCreateDescriptorSetLayout);
DEFINE_VK_PROC(vkCreateEvent);
DEFINE_VK_PROC(vkCreateFence);
DEFINE_VK_PROC(vkCreateFramebuffer);
DEFINE_VK_PROC(vkCreateGraphicsPipelines);
DEFINE_VK_PROC(vkCreateImage);
DEFINE_VK_PROC(vkCreateImageView);
DEFINE_VK_PROC(vkCreatePipelineCache);
DEFINE_VK_PROC(vkCreatePipelineLayout);
DEFINE_VK_PROC(vkCreateQueryPool);
DEFINE_VK_PROC(vkCreateRenderPass);
DEFINE_VK_PROC(vkCreateRenderPass2);
DEFINE_VK_PROC(vkCreateSampler);
DEFINE_VK_PROC(vkCreateSemaphore);
DEFINE_VK_PROC(vkCreateShaderModule);
DEFINE_VK_PROC(vkDestroyBuffer);
DEFINE_VK_PROC(vkDestroyBufferView);
DEFINE_VK_PROC(vkDestroyCommandPool);
DEFINE_VK_PROC(vkDestroyDescriptorPool);
DEFINE_VK_PROC(vkDestroyDescriptorSetLayout);
DEFINE_VK_PROC(vkDestroyEvent);
DEFINE_VK_PROC(vkDestroyFence);
DEFINE_VK_PROC(vkDestroyFramebuffer);
DEFINE_VK_PROC(vkDestroyImage);
DEFINE_VK_PROC(vkDestroyImageView);
DEFINE_VK_PROC(vkDestroyPipeline);
DEFINE_VK_PROC(vkDestroyPipelineCache);
DEFINE_VK_PROC(vkDestroyPipelineLayout);
DEFINE_VK_PROC(vkDestroyQueryPool);
DEFINE_VK_PROC(vkDestroyRenderPass);
DEFINE_VK_PROC(vkDestroySampler);
DEFINE_VK_PROC(vkDestroySemaphore);
DEFINE_VK_PROC(vkDestroyShaderModule);
DEFINE_VK_PROC(vkDeviceWaitIdle);
DEFINE_VK_PROC(vkEndCommandBuffer);
DEFINE_VK_PROC(vkFlushMappedMemoryRanges);
DEFINE_VK_PROC(vkFreeCommandBuffers);
DEFINE_VK_PROC(vkFreeDescriptorSets);
DEFINE_VK_PROC(vkFreeMemory);
DEFINE_VK_PROC(vkGetBufferMemoryRequirements);
DEFINE_VK_PROC(vkGetDeviceMemoryCommitment);
DEFINE_VK_PROC(vkGetDeviceQueue);
DEFINE_VK_PROC(vkGetEventStatus);
DEFINE_VK_PROC(vkGetFenceStatus);
DEFINE_VK_PROC(vkGetImageMemoryRequirements);
DEFINE_VK_PROC(vkGetImageSparseMemoryRequirements);
DEFINE_VK_PROC(vkGetImageSubresourceLayout);
DEFINE_VK_PROC(vkGetPipelineCacheData);
DEFINE_VK_PROC(vkGetQueryPoolResults);
DEFINE_VK_PROC(vkGetRenderAreaGranularity);
DEFINE_VK_PROC(vkInvalidateMappedMemoryRanges);
DEFINE_VK_PROC(vkMapMemory);
DEFINE_VK_PROC(vkMergePipelineCaches);
DEFINE_VK_PROC(vkQueueBindSparse);
DEFINE_VK_PROC(vkQueueSubmit);
DEFINE_VK_PROC(vkQueueWaitIdle);
DEFINE_VK_PROC(vkResetCommandBuffer);
DEFINE_VK_PROC(vkResetCommandPool);
DEFINE_VK_PROC(vkResetDescriptorPool);
DEFINE_VK_PROC(vkResetEvent);
DEFINE_VK_PROC(vkResetFences);
DEFINE_VK_PROC(vkSetEvent);
DEFINE_VK_PROC(vkUnmapMemory);
DEFINE_VK_PROC(vkUpdateDescriptorSets);
DEFINE_VK_PROC(vkWaitForFences);

/* Device Vulkan 1.1 */
DEFINE_VK_PROC(vkBindBufferMemory2);
DEFINE_VK_PROC(vkBindImageMemory2);
DEFINE_VK_PROC(vkCmdDispatchBase);
DEFINE_VK_PROC(vkCmdSetDeviceMask);
DEFINE_VK_PROC(vkCreateDescriptorUpdateTemplate);
DEFINE_VK_PROC(vkCreateSamplerYcbcrConversion);
DEFINE_VK_PROC(vkDestroyDescriptorUpdateTemplate);
DEFINE_VK_PROC(vkDestroySamplerYcbcrConversion);
DEFINE_VK_PROC(vkGetBufferMemoryRequirements2);
DEFINE_VK_PROC(vkGetDescriptorSetLayoutSupport);
DEFINE_VK_PROC(vkGetDeviceGroupPeerMemoryFeatures);
DEFINE_VK_PROC(vkGetDeviceQueue2);
DEFINE_VK_PROC(vkGetImageMemoryRequirements2);
DEFINE_VK_PROC(vkGetImageSparseMemoryRequirements2);
DEFINE_VK_PROC(vkTrimCommandPool);
DEFINE_VK_PROC(vkUpdateDescriptorSetWithTemplate);

/* Device Vulkan 1.2 */
DEFINE_VK_PROC(vkCmdEndRenderPass2);
DEFINE_VK_PROC(vkCmdBeginRenderPass2);
DEFINE_VK_PROC(vkCmdDrawIndexedIndirectCount);
DEFINE_VK_PROC(vkCmdDrawIndirectCount);
DEFINE_VK_PROC(vkCmdNextSubpass2);
DEFINE_VK_PROC(vkGetBufferDeviceAddress);
DEFINE_VK_PROC(vkGetBufferOpaqueCaptureAddress);
DEFINE_VK_PROC(vkGetDeviceMemoryOpaqueCaptureAddress);
DEFINE_VK_PROC(vkGetSemaphoreCounterValue);
DEFINE_VK_PROC(vkResetQueryPool);
DEFINE_VK_PROC(vkSignalSemaphore);
DEFINE_VK_PROC(vkWaitSemaphores);

/* Device Vulkan 1.3 */
DEFINE_VK_PROC(vkCmdBeginRendering);
DEFINE_VK_PROC(vkCmdBindVertexBuffers2);
DEFINE_VK_PROC(vkCmdBlitImage2);
DEFINE_VK_PROC(vkCmdCopyBuffer2);
DEFINE_VK_PROC(vkCmdCopyBufferToImage2);
DEFINE_VK_PROC(vkCmdCopyImage2);
DEFINE_VK_PROC(vkCmdCopyImageToBuffer2);
DEFINE_VK_PROC(vkCmdEndRendering);
DEFINE_VK_PROC(vkCmdPipelineBarrier2);
DEFINE_VK_PROC(vkCmdResetEvent2);
DEFINE_VK_PROC(vkCmdResolveImage2);
DEFINE_VK_PROC(vkCmdSetCullMode);
DEFINE_VK_PROC(vkCmdSetDepthBias);
DEFINE_VK_PROC(vkCmdSetDepthBiasEnable);
DEFINE_VK_PROC(vkCmdSetDepthBoundsTestEnable);
DEFINE_VK_PROC(vkCmdSetDepthCompareOp);
DEFINE_VK_PROC(vkCmdSetDepthTestEnable);
DEFINE_VK_PROC(vkCmdSetDepthWriteEnable);
DEFINE_VK_PROC(vkCmdSetEvent2);
DEFINE_VK_PROC(vkCmdSetFrontFace);
DEFINE_VK_PROC(vkCmdSetPrimitiveRestartEnable);
DEFINE_VK_PROC(vkCmdSetPrimitiveTopology);
DEFINE_VK_PROC(vkCmdSetRasterizerDiscardEnable);
DEFINE_VK_PROC(vkCmdSetScissorWithCount);
DEFINE_VK_PROC(vkCmdSetStencilOp);
DEFINE_VK_PROC(vkCmdSetStencilTestEnable);
DEFINE_VK_PROC(vkCmdSetViewportWithCount);
DEFINE_VK_PROC(vkCmdWaitEvents2);
DEFINE_VK_PROC(vkCmdWriteTimestamp2);
DEFINE_VK_PROC(vkCreatePrivateDataSlot);
DEFINE_VK_PROC(vkDestroyPrivateDataSlot);
DEFINE_VK_PROC(vkGetDeviceBufferMemoryRequirements);
DEFINE_VK_PROC(vkGetDeviceImageMemoryRequirements);
DEFINE_VK_PROC(vkGetDeviceImageSparseMemoryRequirements);
DEFINE_VK_PROC(vkGetPrivateData);
DEFINE_VK_PROC(vkQueueSubmit2);
DEFINE_VK_PROC(vkSetPrivateData);

/* Device Vulkan 1.4 */
DEFINE_VK_PROC(vkCmdBindDescriptorSets2);
DEFINE_VK_PROC(vkCmdBindIndexBuffer2);
DEFINE_VK_PROC(vkCmdPushConstants2);
DEFINE_VK_PROC(vkCmdPushDescriptorSet);
DEFINE_VK_PROC(vkCmdPushDescriptorSet2);
DEFINE_VK_PROC(vkCmdPushDescriptorSetWithTemplate);
DEFINE_VK_PROC(vkCmdPushDescriptorSetWithTemplate2);
DEFINE_VK_PROC(vkCmdSetLineStipple);
DEFINE_VK_PROC(vkCmdSetRenderingAttachmentLocations);
DEFINE_VK_PROC(vkCmdSetRenderingInputAttachmentIndices);
DEFINE_VK_PROC(vkCopyImageToImage);
DEFINE_VK_PROC(vkCopyImageToMemory);
DEFINE_VK_PROC(vkCopyMemoryToImage);
DEFINE_VK_PROC(vkGetDeviceImageSubresourceLayout);
DEFINE_VK_PROC(vkGetImageSubresourceLayout2);
DEFINE_VK_PROC(vkGetRenderingAreaGranularity);
DEFINE_VK_PROC(vkMapMemory2);
DEFINE_VK_PROC(vkTransitionImageLayout);
DEFINE_VK_PROC(vkUnmapMemory2);

#ifdef VP_TARGET_PLATFORM_win32
/* Device VK_KHR_external_memory_win32 */
DEFINE_VK_PROC(vkGetMemoryWin32HandleKHR);
DEFINE_VK_PROC(vkGetMemoryWin32HandlePropertiesKHR);

/* Device VK_KHR_external_semaphore_win32 */
DEFINE_VK_PROC(vkImportSemaphoreWin32HandleKHR);
#endif

#ifdef VP_TARGET_PLATFORM_linux
/* Device VK_KHR_external_memory_fd */
DEFINE_VK_PROC(vkGetMemoryFdKHR);
DEFINE_VK_PROC(vkGetMemoryFdPropertiesKHR);

/* Device VK_KHR_external_semaphore_fd */
DEFINE_VK_PROC(vkImportSemaphoreFdKHR);
DEFINE_VK_PROC(vkGetSemaphoreFdKHR);

/* Device VK_EXT_image_drm_modifier */
DEFINE_VK_PROC(vkGetImageDrmFormatModifierPropertiesEXT);
#endif

#ifdef AWN_VK_DEBUG
    /* Device VK_EXT_device_fault */
    DEFINE_VK_PROC(vkGetDeviceFaultInfoEXT);
#endif

/* Device VK_EXT_external_memory_host*/
DEFINE_VK_PROC(vkGetMemoryHostPointerPropertiesEXT);

/* Device VK_EXT_descriptor_buffer */
DEFINE_VK_PROC(vkCmdBindDescriptorBuffersEXT);
DEFINE_VK_PROC(vkCmdSetDescriptorBufferOffsetsEXT);
DEFINE_VK_PROC(vkGetDescriptorEXT);
DEFINE_VK_PROC(vkGetDescriptorSetLayoutBindingOffsetEXT);
DEFINE_VK_PROC(vkGetDescriptorSetLayoutSizeEXT);
/* + Vulkan 1.4 */
DEFINE_VK_PROC(vkCmdSetDescriptorBufferOffsets2EXT);

/* Device VK_EXT_shader_object*/
DEFINE_VK_PROC(vkCmdBindShadersEXT);
DEFINE_VK_PROC(vkCmdSetAlphaToCoverageEnableEXT);
DEFINE_VK_PROC(vkCmdSetAlphaToOneEnableEXT);
DEFINE_VK_PROC(vkCmdSetColorBlendEnableEXT);
DEFINE_VK_PROC(vkCmdSetColorBlendEquationEXT);
DEFINE_VK_PROC(vkCmdSetColorWriteMaskEXT);
DEFINE_VK_PROC(vkCmdSetDepthClampEnableEXT);
DEFINE_VK_PROC(vkCmdSetLogicOpEXT);
DEFINE_VK_PROC(vkCmdSetLogicOpEnableEXT);
DEFINE_VK_PROC(vkCmdSetPatchControlPointsEXT);
DEFINE_VK_PROC(vkCmdSetPolygonModeEXT);
DEFINE_VK_PROC(vkCmdSetRasterizationSamplesEXT);
DEFINE_VK_PROC(vkCmdSetSampleMaskEXT);
DEFINE_VK_PROC(vkCmdSetVertexInputEXT);
DEFINE_VK_PROC(vkCreateShadersEXT);
DEFINE_VK_PROC(vkDestroyShaderEXT);
DEFINE_VK_PROC(vkGetShaderBinaryDataEXT);

/* + VK_EXT_conservative_rasterization */
DEFINE_VK_PROC(vkCmdSetConservativeRasterizationModeEXT);
DEFINE_VK_PROC(vkCmdSetExtraPrimitiveOverestimationSizeEXT);

/* Device VK_EXT_mesh_shader */
DEFINE_VK_PROC(vkCmdDrawMeshTasksEXT);
DEFINE_VK_PROC(vkCmdDrawMeshTasksIndirectCountEXT);
DEFINE_VK_PROC(vkCmdDrawMeshTasksIndirectEXT);

/* Device VK_EXT_descriptor_heap */
#ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
DEFINE_VK_PROC(vkCmdBindResourceHeapEXT);
DEFINE_VK_PROC(vkCmdBindSamplerHeapEXT);
DEFINE_VK_PROC(vkCmdPushDataEXT);
DEFINE_VK_PROC(vkGetImageOpaqueCaptureDataEXT);
DEFINE_VK_PROC(vkGetPhysicalDeviceDescriptorSizeEXT);
DEFINE_VK_PROC(vkWriteResourceDescriptorsEXT);
DEFINE_VK_PROC(vkWriteSamplerDescriptorsEXT);
#endif

/* Device VK_KHR_device_address_commands */
#ifdef AWN_ENABLE_VK_DEVICE_ADDRESS_COMMANDS
DEFINE_VK_PROC(vkCmdBindIndexBuffer3KHR);
DEFINE_VK_PROC(vkCmdBindVertexBuffers3KHR);
DEFINE_VK_PROC(vkCmdCopyImageToMemoryKHR);
DEFINE_VK_PROC(vkCmdCopyMemoryKHR);
DEFINE_VK_PROC(vkCmdCopyMemoryToImageKHR);
DEFINE_VK_PROC(vkCmdCopyQueryPoolResultsToMemoryKHR);
DEFINE_VK_PROC(vkCmdDispatchIndirect2KHR);
DEFINE_VK_PROC(vkCmdDrawIndexedIndirect2KHR);
DEFINE_VK_PROC(vkCmdDrawIndirect2KHR);
DEFINE_VK_PROC(vkCmdFillMemoryKHR);
DEFINE_VK_PROC(vkCmdUpdateMemoryKHR);
DEFINE_VK_PROC(vkCmdDrawMeshTasksIndirectCount2EXT);
//DEFINE_VK_PROC(vkCmdBeginConditionalRendering2EXT);
//DEFINE_VK_PROC(vkCmdDrawIndexedIndirectCount2KHR);
//DEFINE_VK_PROC(vkCmdDrawIndirectCount2KHR);
#endif

void LoadVkCProcsDevice(VkDevice device) {

    /* Device procs */
    LOAD_DEVICE_VK_PROC(vkAllocateCommandBuffers);
    LOAD_DEVICE_VK_PROC(vkAllocateDescriptorSets);
    LOAD_DEVICE_VK_PROC(vkAllocateMemory);
    LOAD_DEVICE_VK_PROC(vkBeginCommandBuffer);
    LOAD_DEVICE_VK_PROC(vkBindBufferMemory);
    LOAD_DEVICE_VK_PROC(vkBindImageMemory);
    LOAD_DEVICE_VK_PROC(vkCmdBeginQuery);
    LOAD_DEVICE_VK_PROC(vkCmdBeginRenderPass);
    LOAD_DEVICE_VK_PROC(vkCmdBindDescriptorSets);
    LOAD_DEVICE_VK_PROC(vkCmdBindIndexBuffer);
    LOAD_DEVICE_VK_PROC(vkCmdBindPipeline);
    LOAD_DEVICE_VK_PROC(vkCmdBindVertexBuffers);
    LOAD_DEVICE_VK_PROC(vkCmdBlitImage);
    LOAD_DEVICE_VK_PROC(vkCmdClearAttachments);
    LOAD_DEVICE_VK_PROC(vkCmdClearColorImage);
    LOAD_DEVICE_VK_PROC(vkCmdClearDepthStencilImage);
    LOAD_DEVICE_VK_PROC(vkCmdCopyBuffer);
    LOAD_DEVICE_VK_PROC(vkCmdCopyBufferToImage);
    LOAD_DEVICE_VK_PROC(vkCmdCopyImage);
    LOAD_DEVICE_VK_PROC(vkCmdCopyImageToBuffer);
    LOAD_DEVICE_VK_PROC(vkCmdCopyQueryPoolResults);
    LOAD_DEVICE_VK_PROC(vkCmdDispatch);
    LOAD_DEVICE_VK_PROC(vkCmdDispatchIndirect);
    LOAD_DEVICE_VK_PROC(vkCmdDraw);
    LOAD_DEVICE_VK_PROC(vkCmdDrawIndexed);
    LOAD_DEVICE_VK_PROC(vkCmdDrawIndexedIndirect);
    LOAD_DEVICE_VK_PROC(vkCmdDrawIndirect);
    LOAD_DEVICE_VK_PROC(vkCmdEndQuery);
    LOAD_DEVICE_VK_PROC(vkCmdEndRenderPass);
    LOAD_DEVICE_VK_PROC(vkCmdExecuteCommands);
    LOAD_DEVICE_VK_PROC(vkCmdFillBuffer);
    LOAD_DEVICE_VK_PROC(vkCmdNextSubpass);
    LOAD_DEVICE_VK_PROC(vkCmdPipelineBarrier);
    LOAD_DEVICE_VK_PROC(vkCmdPushConstants);
    LOAD_DEVICE_VK_PROC(vkCmdResetEvent);
    LOAD_DEVICE_VK_PROC(vkCmdResetQueryPool);
    LOAD_DEVICE_VK_PROC(vkCmdResolveImage);
    LOAD_DEVICE_VK_PROC(vkCmdSetBlendConstants);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthBounds);
    LOAD_DEVICE_VK_PROC(vkCmdSetEvent);
    LOAD_DEVICE_VK_PROC(vkCmdSetLineWidth);
    LOAD_DEVICE_VK_PROC(vkCmdSetScissor);
    LOAD_DEVICE_VK_PROC(vkCmdSetStencilCompareMask);
    LOAD_DEVICE_VK_PROC(vkCmdSetStencilReference);
    LOAD_DEVICE_VK_PROC(vkCmdSetStencilWriteMask);
    LOAD_DEVICE_VK_PROC(vkCmdSetViewport);
    LOAD_DEVICE_VK_PROC(vkCmdUpdateBuffer);
    LOAD_DEVICE_VK_PROC(vkCmdWaitEvents);
    LOAD_DEVICE_VK_PROC(vkCmdWriteTimestamp);
    LOAD_DEVICE_VK_PROC(vkCreateBuffer);
    LOAD_DEVICE_VK_PROC(vkCreateBufferView);
    LOAD_DEVICE_VK_PROC(vkCreateCommandPool);
    LOAD_DEVICE_VK_PROC(vkCreateComputePipelines);
    LOAD_DEVICE_VK_PROC(vkCreateDescriptorPool);
    LOAD_DEVICE_VK_PROC(vkCreateDescriptorSetLayout);
    LOAD_DEVICE_VK_PROC(vkCreateEvent);
    LOAD_DEVICE_VK_PROC(vkCreateFence);
    LOAD_DEVICE_VK_PROC(vkCreateFramebuffer);
    LOAD_DEVICE_VK_PROC(vkCreateGraphicsPipelines);
    LOAD_DEVICE_VK_PROC(vkCreateImage);
    LOAD_DEVICE_VK_PROC(vkCreateImageView);
    LOAD_DEVICE_VK_PROC(vkCreatePipelineCache);
    LOAD_DEVICE_VK_PROC(vkCreatePipelineLayout);
    LOAD_DEVICE_VK_PROC(vkCreateQueryPool);
    LOAD_DEVICE_VK_PROC(vkCreateRenderPass);
    LOAD_DEVICE_VK_PROC(vkCreateRenderPass2);
    LOAD_DEVICE_VK_PROC(vkCreateSampler);
    LOAD_DEVICE_VK_PROC(vkCreateSemaphore);
    LOAD_DEVICE_VK_PROC(vkCreateShaderModule);
    LOAD_DEVICE_VK_PROC(vkDestroyBuffer);
    LOAD_DEVICE_VK_PROC(vkDestroyBufferView);
    LOAD_DEVICE_VK_PROC(vkDestroyCommandPool);
    LOAD_DEVICE_VK_PROC(vkDestroyDescriptorPool);
    LOAD_DEVICE_VK_PROC(vkDestroyDescriptorSetLayout);
    LOAD_DEVICE_VK_PROC(vkDestroyEvent);
    LOAD_DEVICE_VK_PROC(vkDestroyFence);
    LOAD_DEVICE_VK_PROC(vkDestroyFramebuffer);
    LOAD_DEVICE_VK_PROC(vkDestroyImage);
    LOAD_DEVICE_VK_PROC(vkDestroyImageView);
    LOAD_DEVICE_VK_PROC(vkDestroyPipeline);
    LOAD_DEVICE_VK_PROC(vkDestroyPipelineCache);
    LOAD_DEVICE_VK_PROC(vkDestroyPipelineLayout);
    LOAD_DEVICE_VK_PROC(vkDestroyQueryPool);
    LOAD_DEVICE_VK_PROC(vkDestroyRenderPass);
    LOAD_DEVICE_VK_PROC(vkDestroySampler);
    LOAD_DEVICE_VK_PROC(vkDestroySemaphore);
    LOAD_DEVICE_VK_PROC(vkDestroyShaderModule);
    LOAD_DEVICE_VK_PROC(vkDeviceWaitIdle);
    LOAD_DEVICE_VK_PROC(vkEndCommandBuffer);
    LOAD_DEVICE_VK_PROC(vkFlushMappedMemoryRanges);
    LOAD_DEVICE_VK_PROC(vkFreeCommandBuffers);
    LOAD_DEVICE_VK_PROC(vkFreeDescriptorSets);
    LOAD_DEVICE_VK_PROC(vkFreeMemory);
    LOAD_DEVICE_VK_PROC(vkGetBufferMemoryRequirements);
    LOAD_DEVICE_VK_PROC(vkGetDeviceMemoryCommitment);
    LOAD_DEVICE_VK_PROC(vkGetDeviceQueue);
    LOAD_DEVICE_VK_PROC(vkGetEventStatus);
    LOAD_DEVICE_VK_PROC(vkGetFenceStatus);
    LOAD_DEVICE_VK_PROC(vkGetImageMemoryRequirements);
    LOAD_DEVICE_VK_PROC(vkGetImageSparseMemoryRequirements);
    LOAD_DEVICE_VK_PROC(vkGetImageSubresourceLayout);
    LOAD_DEVICE_VK_PROC(vkGetPipelineCacheData);
    LOAD_DEVICE_VK_PROC(vkGetQueryPoolResults);
    LOAD_DEVICE_VK_PROC(vkGetRenderAreaGranularity);
    LOAD_DEVICE_VK_PROC(vkInvalidateMappedMemoryRanges);
    LOAD_DEVICE_VK_PROC(vkMapMemory);
    LOAD_DEVICE_VK_PROC(vkMergePipelineCaches);
    LOAD_DEVICE_VK_PROC(vkQueueBindSparse);
    LOAD_DEVICE_VK_PROC(vkQueueSubmit);
    LOAD_DEVICE_VK_PROC(vkQueueWaitIdle);
    LOAD_DEVICE_VK_PROC(vkResetCommandBuffer);
    LOAD_DEVICE_VK_PROC(vkResetCommandPool);
    LOAD_DEVICE_VK_PROC(vkResetDescriptorPool);
    LOAD_DEVICE_VK_PROC(vkResetEvent);
    LOAD_DEVICE_VK_PROC(vkResetFences);
    LOAD_DEVICE_VK_PROC(vkSetEvent);
    LOAD_DEVICE_VK_PROC(vkUnmapMemory);
    LOAD_DEVICE_VK_PROC(vkUpdateDescriptorSets);
    LOAD_DEVICE_VK_PROC(vkWaitForFences);

    /* Device Vulkan 1.1 */
    LOAD_DEVICE_VK_PROC(vkBindBufferMemory2);
    LOAD_DEVICE_VK_PROC(vkBindImageMemory2);
    LOAD_DEVICE_VK_PROC(vkCmdDispatchBase);
    LOAD_DEVICE_VK_PROC(vkCmdSetDeviceMask);
    LOAD_DEVICE_VK_PROC(vkCreateDescriptorUpdateTemplate);
    LOAD_DEVICE_VK_PROC(vkCreateSamplerYcbcrConversion);
    LOAD_DEVICE_VK_PROC(vkDestroyDescriptorUpdateTemplate);
    LOAD_DEVICE_VK_PROC(vkDestroySamplerYcbcrConversion);
    LOAD_DEVICE_VK_PROC(vkGetBufferMemoryRequirements2);
    LOAD_DEVICE_VK_PROC(vkGetDescriptorSetLayoutSupport);
    LOAD_DEVICE_VK_PROC(vkGetDeviceGroupPeerMemoryFeatures);
    LOAD_DEVICE_VK_PROC(vkGetDeviceQueue2);
    LOAD_DEVICE_VK_PROC(vkGetImageMemoryRequirements2);
    LOAD_DEVICE_VK_PROC(vkGetImageSparseMemoryRequirements2);
    LOAD_DEVICE_VK_PROC(vkTrimCommandPool);
    LOAD_DEVICE_VK_PROC(vkUpdateDescriptorSetWithTemplate);

    /* Device Vulkan 1.2 */
    LOAD_DEVICE_VK_PROC(vkCmdEndRenderPass2);
    LOAD_DEVICE_VK_PROC(vkCmdBeginRenderPass2);
    LOAD_DEVICE_VK_PROC(vkCmdDrawIndexedIndirectCount);
    LOAD_DEVICE_VK_PROC(vkCmdDrawIndirectCount);
    LOAD_DEVICE_VK_PROC(vkCmdNextSubpass2);
    LOAD_DEVICE_VK_PROC(vkGetBufferDeviceAddress);
    LOAD_DEVICE_VK_PROC(vkGetBufferOpaqueCaptureAddress);
    LOAD_DEVICE_VK_PROC(vkGetDeviceMemoryOpaqueCaptureAddress);
    LOAD_DEVICE_VK_PROC(vkGetSemaphoreCounterValue);
    LOAD_DEVICE_VK_PROC(vkResetQueryPool);
    LOAD_DEVICE_VK_PROC(vkSignalSemaphore);
    LOAD_DEVICE_VK_PROC(vkWaitSemaphores);

    /* Device Vulkan 1.3 */
    LOAD_DEVICE_VK_PROC(vkCmdBeginRendering);
    LOAD_DEVICE_VK_PROC(vkCmdBindVertexBuffers2);
    LOAD_DEVICE_VK_PROC(vkCmdBlitImage2);
    LOAD_DEVICE_VK_PROC(vkCmdCopyBuffer2);
    LOAD_DEVICE_VK_PROC(vkCmdCopyBufferToImage2);
    LOAD_DEVICE_VK_PROC(vkCmdCopyImage2);
    LOAD_DEVICE_VK_PROC(vkCmdCopyImageToBuffer2);
    LOAD_DEVICE_VK_PROC(vkCmdEndRendering);
    LOAD_DEVICE_VK_PROC(vkCmdPipelineBarrier2);
    LOAD_DEVICE_VK_PROC(vkCmdResetEvent2);
    LOAD_DEVICE_VK_PROC(vkCmdResolveImage2);
    LOAD_DEVICE_VK_PROC(vkCmdSetCullMode);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthBias);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthBiasEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthBoundsTestEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthCompareOp);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthTestEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthWriteEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetEvent2);
    LOAD_DEVICE_VK_PROC(vkCmdSetFrontFace);
    LOAD_DEVICE_VK_PROC(vkCmdSetPrimitiveRestartEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetPrimitiveTopology);
    LOAD_DEVICE_VK_PROC(vkCmdSetRasterizerDiscardEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetScissorWithCount);
    LOAD_DEVICE_VK_PROC(vkCmdSetStencilOp);
    LOAD_DEVICE_VK_PROC(vkCmdSetStencilTestEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetViewportWithCount);
    LOAD_DEVICE_VK_PROC(vkCmdWaitEvents2);
    LOAD_DEVICE_VK_PROC(vkCmdWriteTimestamp2);
    LOAD_DEVICE_VK_PROC(vkCreatePrivateDataSlot);
    LOAD_DEVICE_VK_PROC(vkDestroyPrivateDataSlot);
    LOAD_DEVICE_VK_PROC(vkGetDeviceBufferMemoryRequirements);
    LOAD_DEVICE_VK_PROC(vkGetDeviceImageMemoryRequirements);
    LOAD_DEVICE_VK_PROC(vkGetDeviceImageSparseMemoryRequirements);
    LOAD_DEVICE_VK_PROC(vkGetPrivateData);
    LOAD_DEVICE_VK_PROC(vkQueueSubmit2);
    LOAD_DEVICE_VK_PROC(vkSetPrivateData);

    /* Device Vulkan 1.4 */
    LOAD_DEVICE_VK_PROC(vkCmdBindDescriptorSets2);
    LOAD_DEVICE_VK_PROC(vkCmdBindIndexBuffer2);
    LOAD_DEVICE_VK_PROC(vkCmdPushConstants2);
    LOAD_DEVICE_VK_PROC(vkCmdPushDescriptorSet);
    LOAD_DEVICE_VK_PROC(vkCmdPushDescriptorSet2);
    LOAD_DEVICE_VK_PROC(vkCmdPushDescriptorSetWithTemplate);
    LOAD_DEVICE_VK_PROC(vkCmdPushDescriptorSetWithTemplate2);
    LOAD_DEVICE_VK_PROC(vkCmdSetLineStipple);
    LOAD_DEVICE_VK_PROC(vkCmdSetRenderingAttachmentLocations);
    LOAD_DEVICE_VK_PROC(vkCmdSetRenderingInputAttachmentIndices);
    LOAD_DEVICE_VK_PROC(vkCopyImageToImage);
    LOAD_DEVICE_VK_PROC(vkCopyImageToMemory);
    LOAD_DEVICE_VK_PROC(vkCopyMemoryToImage);
    LOAD_DEVICE_VK_PROC(vkGetDeviceImageSubresourceLayout);
    LOAD_DEVICE_VK_PROC(vkGetImageSubresourceLayout2);
    LOAD_DEVICE_VK_PROC(vkGetRenderingAreaGranularity);
    LOAD_DEVICE_VK_PROC(vkMapMemory2);
    LOAD_DEVICE_VK_PROC(vkTransitionImageLayout);
    LOAD_DEVICE_VK_PROC(vkUnmapMemory2);

#ifdef VP_TARGET_PLATFORM_win32
    /* Device VK_KHR_external_memory_win32 */
    LOAD_DEVICE_VK_PROC(vkGetMemoryWin32HandleKHR);
    LOAD_DEVICE_VK_PROC(vkGetMemoryWin32HandlePropertiesKHR);

    /* Device VK_KHR_external_semaphore_win32 */
    LOAD_DEVICE_VK_PROC(vkImportSemaphoreWin32HandleKHR);
#endif

#ifdef VP_TARGET_PLATFORM_linux
    /* Device VK_KHR_external_memory_fd */
    LOAD_DEVICE_VK_PROC(vkGetMemoryFdKHR);
    LOAD_DEVICE_VK_PROC(vkGetMemoryFdPropertiesKHR);

    /* Device VK_KHR_external_semaphore_fd */
    LOAD_DEVICE_VK_PROC(vkImportSemaphoreFdKHR);
    LOAD_DEVICE_VK_PROC(vkGetSemaphoreFdKHR);

    /* Device VK_EXT_image_drm_modifier */
    LOAD_DEVICE_VK_PROC(vkGetImageDrmFormatModifierPropertiesEXT);
#endif

#ifdef AWN_VK_DEBUG
    /* Device VK_EXT_device_fault */
    //LOAD_DEVICE_VK_PROC(vkGetDeviceFaultInfoEXT);
#endif

    /* Device VK_EXT_external_memory_host*/
    LOAD_DEVICE_VK_PROC(vkGetMemoryHostPointerPropertiesEXT);

#ifndef AWN_ENABLE_VK_DESCRIPTOR_HEAP
    /* Device VK_EXT_descriptor_buffer */
    LOAD_DEVICE_VK_PROC(vkCmdBindDescriptorBuffersEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetDescriptorBufferOffsetsEXT);
    LOAD_DEVICE_VK_PROC(vkGetDescriptorEXT);
    LOAD_DEVICE_VK_PROC(vkGetDescriptorSetLayoutBindingOffsetEXT);
    LOAD_DEVICE_VK_PROC(vkGetDescriptorSetLayoutSizeEXT);
    /* + Vulkan 1.4 */
    LOAD_DEVICE_VK_PROC(vkCmdSetDescriptorBufferOffsets2EXT);
#endif

    /* Device VK_EXT_shader_object*/
    LOAD_DEVICE_VK_PROC(vkCmdBindShadersEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetAlphaToCoverageEnableEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetAlphaToOneEnableEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetColorBlendEnableEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetColorBlendEquationEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetColorWriteMaskEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthClampEnableEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetLogicOpEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetLogicOpEnableEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetPatchControlPointsEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetPolygonModeEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetRasterizationSamplesEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetSampleMaskEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetVertexInputEXT);
    LOAD_DEVICE_VK_PROC(vkCreateShadersEXT);
    LOAD_DEVICE_VK_PROC(vkDestroyShaderEXT);
    LOAD_DEVICE_VK_PROC(vkGetShaderBinaryDataEXT);

    /* + VK_EXT_conservative_rasterization */
    LOAD_DEVICE_VK_PROC(vkCmdSetConservativeRasterizationModeEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetExtraPrimitiveOverestimationSizeEXT);

    if (awn::gfx::IsOptionalFeature(awn::gfx::OptionalFeature::SupportsMeshShaders)) {
        /* Device VK_EXT_mesh_shader */
        LOAD_DEVICE_VK_PROC(vkCmdDrawMeshTasksEXT);
        LOAD_DEVICE_VK_PROC(vkCmdDrawMeshTasksIndirectCountEXT);
        LOAD_DEVICE_VK_PROC(vkCmdDrawMeshTasksIndirectEXT);
    }

    /* Device VK_EXT_descriptor_heap */
#ifdef AWN_ENABLE_VK_DESCRIPTOR_HEAP
    LOAD_DEVICE_VK_PROC(vkCmdBindResourceHeapEXT);
    LOAD_DEVICE_VK_PROC(vkCmdBindSamplerHeapEXT);
    LOAD_DEVICE_VK_PROC(vkCmdPushDataEXT);
    //LOAD_DEVICE_VK_PROC(vkGetImageOpaqueCaptureDataEXT);
    //LOAD_DEVICE_VK_PROC(vkGetPhysicalDeviceDescriptorSizeEXT);
    LOAD_DEVICE_VK_PROC(vkWriteResourceDescriptorsEXT);
    LOAD_DEVICE_VK_PROC(vkWriteSamplerDescriptorsEXT);
#endif

    /* Device VK_KHR_device_address_commands */
#ifdef AWN_ENABLE_VK_DEVICE_ADDRESS_COMMANDS
    LOAD_DEVICE_VK_PROC(vkCmdBindIndexBuffer3KHR);
    LOAD_DEVICE_VK_PROC(vkCmdBindVertexBuffers3KHR);
    LOAD_DEVICE_VK_PROC(vkCmdCopyImageToMemoryKHR);
    LOAD_DEVICE_VK_PROC(vkCmdCopyMemoryKHR);
    LOAD_DEVICE_VK_PROC(vkCmdCopyMemoryToImageKHR);
    LOAD_DEVICE_VK_PROC(vkCmdCopyQueryPoolResultsToMemoryKHR);
    LOAD_DEVICE_VK_PROC(vkCmdDispatchIndirect2KHR);
    LOAD_DEVICE_VK_PROC(vkCmdDrawIndexedIndirect2KHR);
    LOAD_DEVICE_VK_PROC(vkCmdDrawIndirect2KHR);
    LOAD_DEVICE_VK_PROC(vkCmdFillMemoryKHR);
    LOAD_DEVICE_VK_PROC(vkCmdUpdateMemoryKHR);
    if (awn::gfx::IsOptionalFeature(awn::gfx::OptionalFeature::SupportsMeshShaders)) {
        LOAD_DEVICE_VK_PROC(vkCmdDrawMeshTasksIndirectCount2EXT);
    }
    //LOAD_DEVICE_VK_PROC(vkCmdBeginConditionalRendering2EXT);
    //LOAD_DEVICE_VK_PROC(vkCmdDrawIndexedIndirectCount2KHR);
    //LOAD_DEVICE_VK_PROC(vkCmdDrawIndirectCount2KHR);
#endif
    return;
}
