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

#define LOAD_INITIAL_VK_PROC(name)  pfn_##name = reinterpret_cast<PFN_##name>(::GetProcAddress(sVulkanDLL, #name)); if (pfn_##name == nullptr) { ProcAbort(#name); };
#define LOAD_INSTANCE_VK_PROC(name) pfn_##name = reinterpret_cast<PFN_##name>(::pfn_vkGetInstanceProcAddr(instance, #name)); if (pfn_##name == nullptr) { ProcAbort(#name); };
#define LOAD_DEVICE_VK_PROC(name)   pfn_##name = reinterpret_cast<PFN_##name>(::pfn_vkGetDeviceProcAddr(device, #name)); if (pfn_##name == nullptr) { ProcAbort(#name); };

HMODULE sVulkanDLL = nullptr;

/* Initial procs */
DEFINE_VK_PROC(vkCreateInstance);
DEFINE_VK_PROC(vkEnumerateInstanceExtensionProperties);
DEFINE_VK_PROC(vkEnumerateInstanceLayerProperties);
DEFINE_VK_PROC(vkEnumerateInstanceVersion);
DEFINE_VK_PROC(vkGetInstanceProcAddr);

#pragma GCC diagnostic ignored "-Wcast-function-type"

void LoadVkCProcsInitial() {

    /* Load vulkan dll */
    HMODULE sVulkanDLL = ::LoadLibraryA("vulkan-1.dll");
    VP_ASSERT(sVulkanDLL != nullptr);

    /* Initial procs */
    LOAD_INITIAL_VK_PROC(vkCreateInstance);
    LOAD_INITIAL_VK_PROC(vkEnumerateInstanceExtensionProperties);
    LOAD_INITIAL_VK_PROC(vkEnumerateInstanceLayerProperties);
    LOAD_INITIAL_VK_PROC(vkEnumerateInstanceVersion);
    LOAD_INITIAL_VK_PROC(vkGetInstanceProcAddr);

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
DEFINE_VK_PROC(vkGetPhysicalDeviceExternalBufferProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceExternalFenceProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceExternalSemaphoreProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceFeatures);
DEFINE_VK_PROC(vkGetPhysicalDeviceFeatures2);
DEFINE_VK_PROC(vkGetPhysicalDeviceFormatProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceFormatProperties2);
DEFINE_VK_PROC(vkGetPhysicalDeviceImageFormatProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceImageFormatProperties2);
DEFINE_VK_PROC(vkGetPhysicalDeviceMemoryProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceMemoryProperties2);
DEFINE_VK_PROC(vkGetPhysicalDeviceProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceProperties2);
DEFINE_VK_PROC(vkGetPhysicalDeviceQueueFamilyProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceQueueFamilyProperties2);
DEFINE_VK_PROC(vkGetPhysicalDeviceSparseImageFormatProperties);
DEFINE_VK_PROC(vkGetPhysicalDeviceSparseImageFormatProperties2);
DEFINE_VK_PROC(vkGetPhysicalDeviceToolProperties);

/* Instance Kronos procs */
DEFINE_VK_PROC(vkCreateWin32SurfaceKHR);
DEFINE_VK_PROC(vkDestroySurfaceKHR);
DEFINE_VK_PROC(vkGetPhysicalDevicePresentRectanglesKHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceSurfaceCapabilities2KHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceSurfaceFormats2KHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);
DEFINE_VK_PROC(vkGetPhysicalDeviceWin32PresentationSupportKHR);

/* Instance extension procs */
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
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceExternalBufferProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceExternalFenceProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceExternalSemaphoreProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceFeatures);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceFeatures2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceFormatProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceFormatProperties2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceImageFormatProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceImageFormatProperties2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceMemoryProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceMemoryProperties2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceProperties2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceQueueFamilyProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceQueueFamilyProperties2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSparseImageFormatProperties);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSparseImageFormatProperties2);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceToolProperties);

    /* Instance Kronos procs */
    LOAD_INSTANCE_VK_PROC(vkCreateWin32SurfaceKHR);
    LOAD_INSTANCE_VK_PROC(vkDestroySurfaceKHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDevicePresentRectanglesKHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSurfaceCapabilities2KHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSurfaceFormats2KHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);
    LOAD_INSTANCE_VK_PROC(vkGetPhysicalDeviceWin32PresentationSupportKHR);

    /* Instance extension procs */
    LOAD_INSTANCE_VK_PROC(vkCreateDebugUtilsMessengerEXT);
    LOAD_INSTANCE_VK_PROC(vkDestroyDebugUtilsMessengerEXT);

    return;
}

/* Device procs */
DEFINE_VK_PROC(vkAllocateCommandBuffers);
DEFINE_VK_PROC(vkAllocateDescriptorSets);
DEFINE_VK_PROC(vkAllocateMemory);
DEFINE_VK_PROC(vkBeginCommandBuffer);
DEFINE_VK_PROC(vkBindBufferMemory);
DEFINE_VK_PROC(vkBindBufferMemory2);
DEFINE_VK_PROC(vkBindImageMemory);
DEFINE_VK_PROC(vkBindImageMemory2);
DEFINE_VK_PROC(vkCmdBeginQuery);
DEFINE_VK_PROC(vkCmdBeginRendering);
DEFINE_VK_PROC(vkCmdBeginRenderPass);
DEFINE_VK_PROC(vkCmdBeginRenderPass2);
DEFINE_VK_PROC(vkCmdBindDescriptorSets);
DEFINE_VK_PROC(vkCmdBindIndexBuffer);
DEFINE_VK_PROC(vkCmdBindPipeline);
DEFINE_VK_PROC(vkCmdBindVertexBuffers);
DEFINE_VK_PROC(vkCmdBindVertexBuffers2);
DEFINE_VK_PROC(vkCmdBlitImage);
DEFINE_VK_PROC(vkCmdBlitImage2);
DEFINE_VK_PROC(vkCmdClearAttachments);
DEFINE_VK_PROC(vkCmdClearColorImage);
DEFINE_VK_PROC(vkCmdClearDepthStencilImage);
DEFINE_VK_PROC(vkCmdCopyBuffer);
DEFINE_VK_PROC(vkCmdCopyBuffer2);
DEFINE_VK_PROC(vkCmdCopyBufferToImage);
DEFINE_VK_PROC(vkCmdCopyBufferToImage2);
DEFINE_VK_PROC(vkCmdCopyImage);
DEFINE_VK_PROC(vkCmdCopyImage2);
DEFINE_VK_PROC(vkCmdCopyImageToBuffer);
DEFINE_VK_PROC(vkCmdCopyImageToBuffer2);
DEFINE_VK_PROC(vkCmdCopyQueryPoolResults);
DEFINE_VK_PROC(vkCmdDispatch);
DEFINE_VK_PROC(vkCmdDispatchBase);
DEFINE_VK_PROC(vkCmdDispatchIndirect);
DEFINE_VK_PROC(vkCmdDraw);
DEFINE_VK_PROC(vkCmdDrawIndexed);
DEFINE_VK_PROC(vkCmdDrawIndexedIndirect);
DEFINE_VK_PROC(vkCmdDrawIndexedIndirectCount);
DEFINE_VK_PROC(vkCmdDrawIndirect);
DEFINE_VK_PROC(vkCmdDrawIndirectCount);
DEFINE_VK_PROC(vkCmdEndQuery);
DEFINE_VK_PROC(vkCmdEndRendering);
DEFINE_VK_PROC(vkCmdEndRenderPass);
DEFINE_VK_PROC(vkCmdEndRenderPass2);
DEFINE_VK_PROC(vkCmdExecuteCommands);
DEFINE_VK_PROC(vkCmdFillBuffer);
DEFINE_VK_PROC(vkCmdNextSubpass);
DEFINE_VK_PROC(vkCmdNextSubpass2);
DEFINE_VK_PROC(vkCmdPipelineBarrier);
DEFINE_VK_PROC(vkCmdPipelineBarrier2);
DEFINE_VK_PROC(vkCmdPushConstants);
DEFINE_VK_PROC(vkCmdResetEvent);
DEFINE_VK_PROC(vkCmdResetEvent2);
DEFINE_VK_PROC(vkCmdResetQueryPool);
DEFINE_VK_PROC(vkCmdResolveImage);
DEFINE_VK_PROC(vkCmdResolveImage2);
DEFINE_VK_PROC(vkCmdSetBlendConstants);
DEFINE_VK_PROC(vkCmdSetCullMode);
DEFINE_VK_PROC(vkCmdSetDepthBias);
DEFINE_VK_PROC(vkCmdSetDepthBiasEnable);
DEFINE_VK_PROC(vkCmdSetDepthBounds);
DEFINE_VK_PROC(vkCmdSetDepthBoundsTestEnable);
DEFINE_VK_PROC(vkCmdSetDepthCompareOp);
DEFINE_VK_PROC(vkCmdSetDepthTestEnable);
DEFINE_VK_PROC(vkCmdSetDepthWriteEnable);
DEFINE_VK_PROC(vkCmdSetDeviceMask);
DEFINE_VK_PROC(vkCmdSetEvent);
DEFINE_VK_PROC(vkCmdSetEvent2);
DEFINE_VK_PROC(vkCmdSetFrontFace);
DEFINE_VK_PROC(vkCmdSetLineWidth);
DEFINE_VK_PROC(vkCmdSetPrimitiveRestartEnable);
DEFINE_VK_PROC(vkCmdSetPrimitiveTopology);
DEFINE_VK_PROC(vkCmdSetRasterizerDiscardEnable);
DEFINE_VK_PROC(vkCmdSetScissor);
DEFINE_VK_PROC(vkCmdSetScissorWithCount);
DEFINE_VK_PROC(vkCmdSetStencilCompareMask);
DEFINE_VK_PROC(vkCmdSetStencilOp);
DEFINE_VK_PROC(vkCmdSetStencilReference);
DEFINE_VK_PROC(vkCmdSetStencilTestEnable);
DEFINE_VK_PROC(vkCmdSetStencilWriteMask);
DEFINE_VK_PROC(vkCmdSetViewport);
DEFINE_VK_PROC(vkCmdSetViewportWithCount);
DEFINE_VK_PROC(vkCmdUpdateBuffer);
DEFINE_VK_PROC(vkCmdWaitEvents);
DEFINE_VK_PROC(vkCmdWaitEvents2);
DEFINE_VK_PROC(vkCmdWriteTimestamp);
DEFINE_VK_PROC(vkCmdWriteTimestamp2);
DEFINE_VK_PROC(vkCreateBuffer);
DEFINE_VK_PROC(vkCreateBufferView);
DEFINE_VK_PROC(vkCreateCommandPool);
DEFINE_VK_PROC(vkCreateComputePipelines);
DEFINE_VK_PROC(vkCreateDescriptorPool);
DEFINE_VK_PROC(vkCreateDescriptorSetLayout);
DEFINE_VK_PROC(vkCreateDescriptorUpdateTemplate);
DEFINE_VK_PROC(vkCreateEvent);
DEFINE_VK_PROC(vkCreateFence);
DEFINE_VK_PROC(vkCreateFramebuffer);
DEFINE_VK_PROC(vkCreateGraphicsPipelines);
DEFINE_VK_PROC(vkCreateImage);
DEFINE_VK_PROC(vkCreateImageView);
DEFINE_VK_PROC(vkCreatePipelineCache);
DEFINE_VK_PROC(vkCreatePipelineLayout);
DEFINE_VK_PROC(vkCreatePrivateDataSlot);
DEFINE_VK_PROC(vkCreateQueryPool);
DEFINE_VK_PROC(vkCreateRenderPass);
DEFINE_VK_PROC(vkCreateRenderPass2);
DEFINE_VK_PROC(vkCreateSampler);
DEFINE_VK_PROC(vkCreateSamplerYcbcrConversion);
DEFINE_VK_PROC(vkCreateSemaphore);
DEFINE_VK_PROC(vkCreateShaderModule);
DEFINE_VK_PROC(vkDestroyBuffer);
DEFINE_VK_PROC(vkDestroyBufferView);
DEFINE_VK_PROC(vkDestroyCommandPool);
DEFINE_VK_PROC(vkDestroyDescriptorPool);
DEFINE_VK_PROC(vkDestroyDescriptorSetLayout);
DEFINE_VK_PROC(vkDestroyDescriptorUpdateTemplate);
DEFINE_VK_PROC(vkDestroyEvent);
DEFINE_VK_PROC(vkDestroyFence);
DEFINE_VK_PROC(vkDestroyFramebuffer);
DEFINE_VK_PROC(vkDestroyImage);
DEFINE_VK_PROC(vkDestroyImageView);
DEFINE_VK_PROC(vkDestroyPipeline);
DEFINE_VK_PROC(vkDestroyPipelineCache);
DEFINE_VK_PROC(vkDestroyPipelineLayout);
DEFINE_VK_PROC(vkDestroyPrivateDataSlot);
DEFINE_VK_PROC(vkDestroyQueryPool);
DEFINE_VK_PROC(vkDestroyRenderPass);
DEFINE_VK_PROC(vkDestroySampler);
DEFINE_VK_PROC(vkDestroySamplerYcbcrConversion);
DEFINE_VK_PROC(vkDestroySemaphore);
DEFINE_VK_PROC(vkDestroyShaderModule);
DEFINE_VK_PROC(vkDeviceWaitIdle);
DEFINE_VK_PROC(vkEndCommandBuffer);
DEFINE_VK_PROC(vkFlushMappedMemoryRanges);
DEFINE_VK_PROC(vkFreeCommandBuffers);
DEFINE_VK_PROC(vkFreeDescriptorSets);
DEFINE_VK_PROC(vkFreeMemory);
DEFINE_VK_PROC(vkGetBufferDeviceAddress);
DEFINE_VK_PROC(vkGetBufferMemoryRequirements);
DEFINE_VK_PROC(vkGetBufferMemoryRequirements2);
DEFINE_VK_PROC(vkGetBufferOpaqueCaptureAddress);
DEFINE_VK_PROC(vkGetDescriptorSetLayoutSupport);
DEFINE_VK_PROC(vkGetDeviceBufferMemoryRequirements);
DEFINE_VK_PROC(vkGetDeviceGroupPeerMemoryFeatures);
DEFINE_VK_PROC(vkGetDeviceImageMemoryRequirements);
DEFINE_VK_PROC(vkGetDeviceImageSparseMemoryRequirements);
DEFINE_VK_PROC(vkGetDeviceMemoryCommitment);
DEFINE_VK_PROC(vkGetDeviceMemoryOpaqueCaptureAddress);
DEFINE_VK_PROC(vkGetDeviceQueue);
DEFINE_VK_PROC(vkGetDeviceQueue2);
DEFINE_VK_PROC(vkGetEventStatus);
DEFINE_VK_PROC(vkGetFenceStatus);
DEFINE_VK_PROC(vkGetImageMemoryRequirements);
DEFINE_VK_PROC(vkGetImageMemoryRequirements2);
DEFINE_VK_PROC(vkGetImageSparseMemoryRequirements);
DEFINE_VK_PROC(vkGetImageSparseMemoryRequirements2);
DEFINE_VK_PROC(vkGetImageSubresourceLayout);
DEFINE_VK_PROC(vkGetPipelineCacheData);
DEFINE_VK_PROC(vkGetPrivateData);
DEFINE_VK_PROC(vkGetQueryPoolResults);
DEFINE_VK_PROC(vkGetRenderAreaGranularity);
DEFINE_VK_PROC(vkGetSemaphoreCounterValue);
DEFINE_VK_PROC(vkInvalidateMappedMemoryRanges);
DEFINE_VK_PROC(vkMapMemory);
DEFINE_VK_PROC(vkMergePipelineCaches);
DEFINE_VK_PROC(vkQueueBindSparse);
DEFINE_VK_PROC(vkQueueSubmit);
DEFINE_VK_PROC(vkQueueSubmit2);
DEFINE_VK_PROC(vkQueueWaitIdle);
DEFINE_VK_PROC(vkResetCommandBuffer);
DEFINE_VK_PROC(vkResetCommandPool);
DEFINE_VK_PROC(vkResetDescriptorPool);
DEFINE_VK_PROC(vkResetEvent);
DEFINE_VK_PROC(vkResetFences);
DEFINE_VK_PROC(vkResetQueryPool);
DEFINE_VK_PROC(vkSetEvent);
DEFINE_VK_PROC(vkSetPrivateData);
DEFINE_VK_PROC(vkSignalSemaphore);
DEFINE_VK_PROC(vkTrimCommandPool);
DEFINE_VK_PROC(vkUnmapMemory);
DEFINE_VK_PROC(vkUpdateDescriptorSets);
DEFINE_VK_PROC(vkUpdateDescriptorSetWithTemplate);
DEFINE_VK_PROC(vkWaitForFences);
DEFINE_VK_PROC(vkWaitSemaphores);

/* Device Kronos procs */
DEFINE_VK_PROC(vkAcquireNextImage2KHR);
DEFINE_VK_PROC(vkAcquireNextImageKHR);
DEFINE_VK_PROC(vkCreateSwapchainKHR);
DEFINE_VK_PROC(vkDestroySwapchainKHR);
DEFINE_VK_PROC(vkGetDeviceGroupPresentCapabilitiesKHR);
DEFINE_VK_PROC(vkGetDeviceGroupSurfacePresentModesKHR);
DEFINE_VK_PROC(vkGetImageSubresourceLayout2KHR);
DEFINE_VK_PROC(vkGetSwapchainImagesKHR);
DEFINE_VK_PROC(vkMapMemory2KHR);
DEFINE_VK_PROC(vkQueuePresentKHR);
DEFINE_VK_PROC(vkCmdPushDescriptorSet2KHR);
DEFINE_VK_PROC(vkCmdPushConstants2KHR);

/* Device extension procs */
DEFINE_VK_PROC(vkCmdBindDescriptorBuffersEXT);
DEFINE_VK_PROC(vkCmdBindShadersEXT);
DEFINE_VK_PROC(vkCmdDrawMeshTasksEXT);
DEFINE_VK_PROC(vkCmdSetAlphaToCoverageEnableEXT);
DEFINE_VK_PROC(vkCmdSetColorBlendEnableEXT);
DEFINE_VK_PROC(vkCmdSetColorBlendEquationEXT);
DEFINE_VK_PROC(vkCmdSetColorWriteMaskEXT);
DEFINE_VK_PROC(vkCmdSetConservativeRasterizationModeEXT);
DEFINE_VK_PROC(vkCmdSetDepthClampEnableEXT);
DEFINE_VK_PROC(vkCmdSetExtraPrimitiveOverestimationSizeEXT);
DEFINE_VK_PROC(vkCmdSetLogicOpEXT);
DEFINE_VK_PROC(vkCmdSetLogicOpEnableEXT);
DEFINE_VK_PROC(vkCmdSetPolygonModeEXT);
DEFINE_VK_PROC(vkCmdSetRasterizationSamplesEXT);
DEFINE_VK_PROC(vkCmdSetSampleMaskEXT);
DEFINE_VK_PROC(vkCmdSetVertexInputEXT);
DEFINE_VK_PROC(vkCopyImageToImageEXT);
DEFINE_VK_PROC(vkCopyImageToMemoryEXT);
DEFINE_VK_PROC(vkCopyMemoryToImageEXT);
DEFINE_VK_PROC(vkCreateShadersEXT);
DEFINE_VK_PROC(vkDestroyShaderEXT);
DEFINE_VK_PROC(vkGetDescriptorEXT);
DEFINE_VK_PROC(vkGetDescriptorSetLayoutSizeEXT);
DEFINE_VK_PROC(vkGetMemoryHostPointerPropertiesEXT);
DEFINE_VK_PROC(vkTransitionImageLayoutEXT);

void LoadVkCProcsDevice(VkDevice device) {

    /* Device procs */
    LOAD_DEVICE_VK_PROC(vkAllocateCommandBuffers);
    LOAD_DEVICE_VK_PROC(vkAllocateDescriptorSets);
    LOAD_DEVICE_VK_PROC(vkAllocateMemory);
    LOAD_DEVICE_VK_PROC(vkBeginCommandBuffer);
    LOAD_DEVICE_VK_PROC(vkBindBufferMemory);
    LOAD_DEVICE_VK_PROC(vkBindBufferMemory2);
    LOAD_DEVICE_VK_PROC(vkBindImageMemory);
    LOAD_DEVICE_VK_PROC(vkBindImageMemory2);
    LOAD_DEVICE_VK_PROC(vkCmdBeginQuery);
    LOAD_DEVICE_VK_PROC(vkCmdBeginRendering);
    LOAD_DEVICE_VK_PROC(vkCmdBeginRenderPass);
    LOAD_DEVICE_VK_PROC(vkCmdBeginRenderPass2);
    LOAD_DEVICE_VK_PROC(vkCmdBindDescriptorSets);
    LOAD_DEVICE_VK_PROC(vkCmdBindIndexBuffer);
    LOAD_DEVICE_VK_PROC(vkCmdBindPipeline);
    LOAD_DEVICE_VK_PROC(vkCmdBindVertexBuffers);
    LOAD_DEVICE_VK_PROC(vkCmdBindVertexBuffers2);
    LOAD_DEVICE_VK_PROC(vkCmdBlitImage);
    LOAD_DEVICE_VK_PROC(vkCmdBlitImage2);
    LOAD_DEVICE_VK_PROC(vkCmdClearAttachments);
    LOAD_DEVICE_VK_PROC(vkCmdClearColorImage);
    LOAD_DEVICE_VK_PROC(vkCmdClearDepthStencilImage);
    LOAD_DEVICE_VK_PROC(vkCmdCopyBuffer);
    LOAD_DEVICE_VK_PROC(vkCmdCopyBuffer2);
    LOAD_DEVICE_VK_PROC(vkCmdCopyBufferToImage);
    LOAD_DEVICE_VK_PROC(vkCmdCopyBufferToImage2);
    LOAD_DEVICE_VK_PROC(vkCmdCopyImage);
    LOAD_DEVICE_VK_PROC(vkCmdCopyImage2);
    LOAD_DEVICE_VK_PROC(vkCmdCopyImageToBuffer);
    LOAD_DEVICE_VK_PROC(vkCmdCopyImageToBuffer2);
    LOAD_DEVICE_VK_PROC(vkCmdCopyQueryPoolResults);
    LOAD_DEVICE_VK_PROC(vkCmdDispatch);
    LOAD_DEVICE_VK_PROC(vkCmdDispatchBase);
    LOAD_DEVICE_VK_PROC(vkCmdDispatchIndirect);
    LOAD_DEVICE_VK_PROC(vkCmdDraw);
    LOAD_DEVICE_VK_PROC(vkCmdDrawIndexed);
    LOAD_DEVICE_VK_PROC(vkCmdDrawIndexedIndirect);
    LOAD_DEVICE_VK_PROC(vkCmdDrawIndexedIndirectCount);
    LOAD_DEVICE_VK_PROC(vkCmdDrawIndirect);
    LOAD_DEVICE_VK_PROC(vkCmdDrawIndirectCount);
    LOAD_DEVICE_VK_PROC(vkCmdEndQuery);
    LOAD_DEVICE_VK_PROC(vkCmdEndRendering);
    LOAD_DEVICE_VK_PROC(vkCmdEndRenderPass);
    LOAD_DEVICE_VK_PROC(vkCmdEndRenderPass2);
    LOAD_DEVICE_VK_PROC(vkCmdExecuteCommands);
    LOAD_DEVICE_VK_PROC(vkCmdFillBuffer);
    LOAD_DEVICE_VK_PROC(vkCmdNextSubpass);
    LOAD_DEVICE_VK_PROC(vkCmdNextSubpass2);
    LOAD_DEVICE_VK_PROC(vkCmdPipelineBarrier);
    LOAD_DEVICE_VK_PROC(vkCmdPipelineBarrier2);
    LOAD_DEVICE_VK_PROC(vkCmdPushConstants);
    LOAD_DEVICE_VK_PROC(vkCmdResetEvent);
    LOAD_DEVICE_VK_PROC(vkCmdResetEvent2);
    LOAD_DEVICE_VK_PROC(vkCmdResetQueryPool);
    LOAD_DEVICE_VK_PROC(vkCmdResolveImage);
    LOAD_DEVICE_VK_PROC(vkCmdResolveImage2);
    LOAD_DEVICE_VK_PROC(vkCmdSetBlendConstants);
    LOAD_DEVICE_VK_PROC(vkCmdSetCullMode);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthBias);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthBiasEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthBounds);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthBoundsTestEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthCompareOp);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthTestEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthWriteEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetDeviceMask);
    LOAD_DEVICE_VK_PROC(vkCmdSetEvent);
    LOAD_DEVICE_VK_PROC(vkCmdSetEvent2);
    LOAD_DEVICE_VK_PROC(vkCmdSetFrontFace);
    LOAD_DEVICE_VK_PROC(vkCmdSetLineWidth);
    LOAD_DEVICE_VK_PROC(vkCmdSetPrimitiveRestartEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetPrimitiveTopology);
    LOAD_DEVICE_VK_PROC(vkCmdSetRasterizerDiscardEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetScissor);
    LOAD_DEVICE_VK_PROC(vkCmdSetScissorWithCount);
    LOAD_DEVICE_VK_PROC(vkCmdSetStencilCompareMask);
    LOAD_DEVICE_VK_PROC(vkCmdSetStencilOp);
    LOAD_DEVICE_VK_PROC(vkCmdSetStencilReference);
    LOAD_DEVICE_VK_PROC(vkCmdSetStencilTestEnable);
    LOAD_DEVICE_VK_PROC(vkCmdSetStencilWriteMask);
    LOAD_DEVICE_VK_PROC(vkCmdSetViewport);
    LOAD_DEVICE_VK_PROC(vkCmdSetViewportWithCount);
    LOAD_DEVICE_VK_PROC(vkCmdUpdateBuffer);
    LOAD_DEVICE_VK_PROC(vkCmdWaitEvents);
    LOAD_DEVICE_VK_PROC(vkCmdWaitEvents2);
    LOAD_DEVICE_VK_PROC(vkCmdWriteTimestamp);
    LOAD_DEVICE_VK_PROC(vkCmdWriteTimestamp2);
    LOAD_DEVICE_VK_PROC(vkCreateBuffer);
    LOAD_DEVICE_VK_PROC(vkCreateBufferView);
    LOAD_DEVICE_VK_PROC(vkCreateCommandPool);
    LOAD_DEVICE_VK_PROC(vkCreateComputePipelines);
    LOAD_DEVICE_VK_PROC(vkCreateDescriptorPool);
    LOAD_DEVICE_VK_PROC(vkCreateDescriptorSetLayout);
    LOAD_DEVICE_VK_PROC(vkCreateDescriptorUpdateTemplate);
    LOAD_DEVICE_VK_PROC(vkCreateEvent);
    LOAD_DEVICE_VK_PROC(vkCreateFence);
    LOAD_DEVICE_VK_PROC(vkCreateFramebuffer);
    LOAD_DEVICE_VK_PROC(vkCreateGraphicsPipelines);
    LOAD_DEVICE_VK_PROC(vkCreateImage);
    LOAD_DEVICE_VK_PROC(vkCreateImageView);
    LOAD_DEVICE_VK_PROC(vkCreatePipelineCache);
    LOAD_DEVICE_VK_PROC(vkCreatePipelineLayout);
    LOAD_DEVICE_VK_PROC(vkCreatePrivateDataSlot);
    LOAD_DEVICE_VK_PROC(vkCreateQueryPool);
    LOAD_DEVICE_VK_PROC(vkCreateRenderPass);
    LOAD_DEVICE_VK_PROC(vkCreateRenderPass2);
    LOAD_DEVICE_VK_PROC(vkCreateSampler);
    LOAD_DEVICE_VK_PROC(vkCreateSamplerYcbcrConversion);
    LOAD_DEVICE_VK_PROC(vkCreateSemaphore);
    LOAD_DEVICE_VK_PROC(vkCreateShaderModule);
    LOAD_DEVICE_VK_PROC(vkDestroyBuffer);
    LOAD_DEVICE_VK_PROC(vkDestroyBufferView);
    LOAD_DEVICE_VK_PROC(vkDestroyCommandPool);
    LOAD_DEVICE_VK_PROC(vkDestroyDescriptorPool);
    LOAD_DEVICE_VK_PROC(vkDestroyDescriptorSetLayout);
    LOAD_DEVICE_VK_PROC(vkDestroyDescriptorUpdateTemplate);
    LOAD_DEVICE_VK_PROC(vkDestroyEvent);
    LOAD_DEVICE_VK_PROC(vkDestroyFence);
    LOAD_DEVICE_VK_PROC(vkDestroyFramebuffer);
    LOAD_DEVICE_VK_PROC(vkDestroyImage);
    LOAD_DEVICE_VK_PROC(vkDestroyImageView);
    LOAD_DEVICE_VK_PROC(vkDestroyPipeline);
    LOAD_DEVICE_VK_PROC(vkDestroyPipelineCache);
    LOAD_DEVICE_VK_PROC(vkDestroyPipelineLayout);
    LOAD_DEVICE_VK_PROC(vkDestroyPrivateDataSlot);
    LOAD_DEVICE_VK_PROC(vkDestroyQueryPool);
    LOAD_DEVICE_VK_PROC(vkDestroyRenderPass);
    LOAD_DEVICE_VK_PROC(vkDestroySampler);
    LOAD_DEVICE_VK_PROC(vkDestroySamplerYcbcrConversion);
    LOAD_DEVICE_VK_PROC(vkDestroySemaphore);
    LOAD_DEVICE_VK_PROC(vkDestroyShaderModule);
    LOAD_DEVICE_VK_PROC(vkDeviceWaitIdle);
    LOAD_DEVICE_VK_PROC(vkEndCommandBuffer);
    LOAD_DEVICE_VK_PROC(vkFlushMappedMemoryRanges);
    LOAD_DEVICE_VK_PROC(vkFreeCommandBuffers);
    LOAD_DEVICE_VK_PROC(vkFreeDescriptorSets);
    LOAD_DEVICE_VK_PROC(vkFreeMemory);
    LOAD_DEVICE_VK_PROC(vkGetBufferDeviceAddress);
    LOAD_DEVICE_VK_PROC(vkGetBufferMemoryRequirements);
    LOAD_DEVICE_VK_PROC(vkGetBufferMemoryRequirements2);
    LOAD_DEVICE_VK_PROC(vkGetBufferOpaqueCaptureAddress);
    LOAD_DEVICE_VK_PROC(vkGetDescriptorSetLayoutSupport);
    LOAD_DEVICE_VK_PROC(vkGetDeviceBufferMemoryRequirements);
    LOAD_DEVICE_VK_PROC(vkGetDeviceGroupPeerMemoryFeatures);
    LOAD_DEVICE_VK_PROC(vkGetDeviceImageMemoryRequirements);
    LOAD_DEVICE_VK_PROC(vkGetDeviceImageSparseMemoryRequirements);
    LOAD_DEVICE_VK_PROC(vkGetDeviceMemoryCommitment);
    LOAD_DEVICE_VK_PROC(vkGetDeviceMemoryOpaqueCaptureAddress);
    LOAD_DEVICE_VK_PROC(vkGetDeviceQueue);
    LOAD_DEVICE_VK_PROC(vkGetDeviceQueue2);
    LOAD_DEVICE_VK_PROC(vkGetEventStatus);
    LOAD_DEVICE_VK_PROC(vkGetFenceStatus);
    LOAD_DEVICE_VK_PROC(vkGetImageMemoryRequirements);
    LOAD_DEVICE_VK_PROC(vkGetImageMemoryRequirements2);
    LOAD_DEVICE_VK_PROC(vkGetImageSparseMemoryRequirements);
    LOAD_DEVICE_VK_PROC(vkGetImageSparseMemoryRequirements2);
    LOAD_DEVICE_VK_PROC(vkGetImageSubresourceLayout);
    LOAD_DEVICE_VK_PROC(vkGetPipelineCacheData);
    LOAD_DEVICE_VK_PROC(vkGetPrivateData);
    LOAD_DEVICE_VK_PROC(vkGetQueryPoolResults);
    LOAD_DEVICE_VK_PROC(vkGetRenderAreaGranularity);
    LOAD_DEVICE_VK_PROC(vkGetSemaphoreCounterValue);
    LOAD_DEVICE_VK_PROC(vkInvalidateMappedMemoryRanges);
    LOAD_DEVICE_VK_PROC(vkMapMemory);
    LOAD_DEVICE_VK_PROC(vkMergePipelineCaches);
    LOAD_DEVICE_VK_PROC(vkQueueBindSparse);
    LOAD_DEVICE_VK_PROC(vkQueueSubmit);
    LOAD_DEVICE_VK_PROC(vkQueueSubmit2);
    LOAD_DEVICE_VK_PROC(vkQueueWaitIdle);
    LOAD_DEVICE_VK_PROC(vkResetCommandBuffer);
    LOAD_DEVICE_VK_PROC(vkResetCommandPool);
    LOAD_DEVICE_VK_PROC(vkResetDescriptorPool);
    LOAD_DEVICE_VK_PROC(vkResetEvent);
    LOAD_DEVICE_VK_PROC(vkResetFences);
    LOAD_DEVICE_VK_PROC(vkResetQueryPool);
    LOAD_DEVICE_VK_PROC(vkSetEvent);
    LOAD_DEVICE_VK_PROC(vkSetPrivateData);
    LOAD_DEVICE_VK_PROC(vkSignalSemaphore);
    LOAD_DEVICE_VK_PROC(vkTrimCommandPool);
    LOAD_DEVICE_VK_PROC(vkUnmapMemory);
    LOAD_DEVICE_VK_PROC(vkUpdateDescriptorSets);
    LOAD_DEVICE_VK_PROC(vkUpdateDescriptorSetWithTemplate);
    LOAD_DEVICE_VK_PROC(vkWaitForFences);
    LOAD_DEVICE_VK_PROC(vkWaitSemaphores);

    /* Device Kronos procs */
    LOAD_DEVICE_VK_PROC(vkAcquireNextImage2KHR);
    LOAD_DEVICE_VK_PROC(vkAcquireNextImageKHR);
    LOAD_DEVICE_VK_PROC(vkCreateSwapchainKHR);
    LOAD_DEVICE_VK_PROC(vkDestroySwapchainKHR);
    LOAD_DEVICE_VK_PROC(vkGetDeviceGroupPresentCapabilitiesKHR);
    LOAD_DEVICE_VK_PROC(vkGetDeviceGroupSurfacePresentModesKHR);
    //LOAD_DEVICE_VK_PROC(vkGetImageSubresourceLayout2KHR);
    LOAD_DEVICE_VK_PROC(vkGetSwapchainImagesKHR);
    LOAD_DEVICE_VK_PROC(vkMapMemory2KHR);
    LOAD_DEVICE_VK_PROC(vkQueuePresentKHR);
    LOAD_DEVICE_VK_PROC(vkCmdPushDescriptorSet2KHR);
    LOAD_DEVICE_VK_PROC(vkCmdPushConstants2KHR);

    /* Device extension procs */
    LOAD_DEVICE_VK_PROC(vkCmdBindDescriptorBuffersEXT);
    LOAD_DEVICE_VK_PROC(vkCmdBindShadersEXT);
    LOAD_DEVICE_VK_PROC(vkCmdDrawMeshTasksEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetAlphaToCoverageEnableEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetColorBlendEnableEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetColorBlendEquationEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetColorWriteMaskEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetConservativeRasterizationModeEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetDepthClampEnableEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetExtraPrimitiveOverestimationSizeEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetLogicOpEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetLogicOpEnableEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetPolygonModeEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetRasterizationSamplesEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetSampleMaskEXT);
    LOAD_DEVICE_VK_PROC(vkCmdSetVertexInputEXT);
    LOAD_DEVICE_VK_PROC(vkCopyImageToImageEXT);
    LOAD_DEVICE_VK_PROC(vkCopyImageToMemoryEXT);
    LOAD_DEVICE_VK_PROC(vkCopyMemoryToImageEXT);
    LOAD_DEVICE_VK_PROC(vkCreateShadersEXT);
    LOAD_DEVICE_VK_PROC(vkDestroyShaderEXT);
    LOAD_DEVICE_VK_PROC(vkGetDescriptorEXT);
    LOAD_DEVICE_VK_PROC(vkGetDescriptorSetLayoutSizeEXT);
    LOAD_DEVICE_VK_PROC(vkGetMemoryHostPointerPropertiesEXT);
    LOAD_DEVICE_VK_PROC(vkTransitionImageLayoutEXT);

    return;
}
