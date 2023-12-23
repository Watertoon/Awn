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
#pragma once

#define DEFINE_EXTERN_VK_PROC(name); extern PFN_##name pfn_##name

/* Initial procs */
DEFINE_EXTERN_VK_PROC(vkCreateInstance);
DEFINE_EXTERN_VK_PROC(vkEnumerateInstanceExtensionProperties);
DEFINE_EXTERN_VK_PROC(vkEnumerateInstanceLayerProperties);
DEFINE_EXTERN_VK_PROC(vkEnumerateInstanceVersion);
DEFINE_EXTERN_VK_PROC(vkGetInstanceProcAddr);

void LoadVkCProcsInitial();

/* Instance procs */
DEFINE_EXTERN_VK_PROC(vkCreateDevice);
DEFINE_EXTERN_VK_PROC(vkDestroyDevice);
DEFINE_EXTERN_VK_PROC(vkDestroyInstance);
DEFINE_EXTERN_VK_PROC(vkEnumerateDeviceExtensionProperties);
DEFINE_EXTERN_VK_PROC(vkEnumerateDeviceLayerProperties);
DEFINE_EXTERN_VK_PROC(vkEnumeratePhysicalDeviceGroups);
DEFINE_EXTERN_VK_PROC(vkEnumeratePhysicalDevices);
DEFINE_EXTERN_VK_PROC(vkGetDeviceProcAddr);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceExternalBufferProperties);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceExternalFenceProperties);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceExternalSemaphoreProperties);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceFeatures);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceFeatures2);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceFormatProperties);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceFormatProperties2);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceImageFormatProperties);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceImageFormatProperties2);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceMemoryProperties);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceMemoryProperties2);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceProperties);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceProperties2);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceQueueFamilyProperties);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceQueueFamilyProperties2);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceSparseImageFormatProperties);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceSparseImageFormatProperties2);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceToolProperties);

/* Instance Kronos procs */
DEFINE_EXTERN_VK_PROC(vkCreateWin32SurfaceKHR);
DEFINE_EXTERN_VK_PROC(vkDestroySurfaceKHR);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDevicePresentRectanglesKHR);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceSurfaceCapabilities2KHR);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceSurfaceFormats2KHR);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);
DEFINE_EXTERN_VK_PROC(vkGetPhysicalDeviceWin32PresentationSupportKHR);

/* Instance extension procs */
DEFINE_EXTERN_VK_PROC(vkCreateDebugUtilsMessengerEXT);
DEFINE_EXTERN_VK_PROC(vkDestroyDebugUtilsMessengerEXT);

void LoadVkCProcsInstance(VkInstance instance);

/* Device procs */
DEFINE_EXTERN_VK_PROC(vkAllocateCommandBuffers);
DEFINE_EXTERN_VK_PROC(vkAllocateDescriptorSets);
DEFINE_EXTERN_VK_PROC(vkAllocateMemory);
DEFINE_EXTERN_VK_PROC(vkBeginCommandBuffer);
DEFINE_EXTERN_VK_PROC(vkBindBufferMemory);
DEFINE_EXTERN_VK_PROC(vkBindBufferMemory2);
DEFINE_EXTERN_VK_PROC(vkBindImageMemory);
DEFINE_EXTERN_VK_PROC(vkBindImageMemory2);
DEFINE_EXTERN_VK_PROC(vkCmdBeginQuery);
DEFINE_EXTERN_VK_PROC(vkCmdBeginRendering);
DEFINE_EXTERN_VK_PROC(vkCmdBeginRenderPass);
DEFINE_EXTERN_VK_PROC(vkCmdBeginRenderPass2);
DEFINE_EXTERN_VK_PROC(vkCmdBindDescriptorSets);
DEFINE_EXTERN_VK_PROC(vkCmdBindIndexBuffer);
DEFINE_EXTERN_VK_PROC(vkCmdBindPipeline);
DEFINE_EXTERN_VK_PROC(vkCmdBindVertexBuffers);
DEFINE_EXTERN_VK_PROC(vkCmdBindVertexBuffers2);
DEFINE_EXTERN_VK_PROC(vkCmdBlitImage);
DEFINE_EXTERN_VK_PROC(vkCmdBlitImage2);
DEFINE_EXTERN_VK_PROC(vkCmdClearAttachments);
DEFINE_EXTERN_VK_PROC(vkCmdClearColorImage);
DEFINE_EXTERN_VK_PROC(vkCmdClearDepthStencilImage);
DEFINE_EXTERN_VK_PROC(vkCmdCopyBuffer);
DEFINE_EXTERN_VK_PROC(vkCmdCopyBuffer2);
DEFINE_EXTERN_VK_PROC(vkCmdCopyBufferToImage);
DEFINE_EXTERN_VK_PROC(vkCmdCopyBufferToImage2);
DEFINE_EXTERN_VK_PROC(vkCmdCopyImage);
DEFINE_EXTERN_VK_PROC(vkCmdCopyImage2);
DEFINE_EXTERN_VK_PROC(vkCmdCopyImageToBuffer);
DEFINE_EXTERN_VK_PROC(vkCmdCopyImageToBuffer2);
DEFINE_EXTERN_VK_PROC(vkCmdCopyQueryPoolResults);
DEFINE_EXTERN_VK_PROC(vkCmdDispatch);
DEFINE_EXTERN_VK_PROC(vkCmdDispatchBase);
DEFINE_EXTERN_VK_PROC(vkCmdDispatchIndirect);
DEFINE_EXTERN_VK_PROC(vkCmdDraw);
DEFINE_EXTERN_VK_PROC(vkCmdDrawIndexed);
DEFINE_EXTERN_VK_PROC(vkCmdDrawIndexedIndirect);
DEFINE_EXTERN_VK_PROC(vkCmdDrawIndexedIndirectCount);
DEFINE_EXTERN_VK_PROC(vkCmdDrawIndirect);
DEFINE_EXTERN_VK_PROC(vkCmdDrawIndirectCount);
DEFINE_EXTERN_VK_PROC(vkCmdEndQuery);
DEFINE_EXTERN_VK_PROC(vkCmdEndRendering);
DEFINE_EXTERN_VK_PROC(vkCmdEndRenderPass);
DEFINE_EXTERN_VK_PROC(vkCmdEndRenderPass2);
DEFINE_EXTERN_VK_PROC(vkCmdExecuteCommands);
DEFINE_EXTERN_VK_PROC(vkCmdFillBuffer);
DEFINE_EXTERN_VK_PROC(vkCmdNextSubpass);
DEFINE_EXTERN_VK_PROC(vkCmdNextSubpass2);
DEFINE_EXTERN_VK_PROC(vkCmdPipelineBarrier);
DEFINE_EXTERN_VK_PROC(vkCmdPipelineBarrier2);
DEFINE_EXTERN_VK_PROC(vkCmdPushConstants);
DEFINE_EXTERN_VK_PROC(vkCmdResetEvent);
DEFINE_EXTERN_VK_PROC(vkCmdResetEvent2);
DEFINE_EXTERN_VK_PROC(vkCmdResetQueryPool);
DEFINE_EXTERN_VK_PROC(vkCmdResolveImage);
DEFINE_EXTERN_VK_PROC(vkCmdResolveImage2);
DEFINE_EXTERN_VK_PROC(vkCmdSetBlendConstants);
DEFINE_EXTERN_VK_PROC(vkCmdSetCullMode);
DEFINE_EXTERN_VK_PROC(vkCmdSetDepthBias);
DEFINE_EXTERN_VK_PROC(vkCmdSetDepthBiasEnable);
DEFINE_EXTERN_VK_PROC(vkCmdSetDepthBounds);
DEFINE_EXTERN_VK_PROC(vkCmdSetDepthBoundsTestEnable);
DEFINE_EXTERN_VK_PROC(vkCmdSetDepthCompareOp);
DEFINE_EXTERN_VK_PROC(vkCmdSetDepthTestEnable);
DEFINE_EXTERN_VK_PROC(vkCmdSetDepthWriteEnable);
DEFINE_EXTERN_VK_PROC(vkCmdSetDeviceMask);
DEFINE_EXTERN_VK_PROC(vkCmdSetEvent);
DEFINE_EXTERN_VK_PROC(vkCmdSetEvent2);
DEFINE_EXTERN_VK_PROC(vkCmdSetFrontFace);
DEFINE_EXTERN_VK_PROC(vkCmdSetLineWidth);
DEFINE_EXTERN_VK_PROC(vkCmdSetPrimitiveRestartEnable);
DEFINE_EXTERN_VK_PROC(vkCmdSetPrimitiveTopology);
DEFINE_EXTERN_VK_PROC(vkCmdSetRasterizerDiscardEnable);
DEFINE_EXTERN_VK_PROC(vkCmdSetScissor);
DEFINE_EXTERN_VK_PROC(vkCmdSetScissorWithCount);
DEFINE_EXTERN_VK_PROC(vkCmdSetStencilCompareMask);
DEFINE_EXTERN_VK_PROC(vkCmdSetStencilOp);
DEFINE_EXTERN_VK_PROC(vkCmdSetStencilReference);
DEFINE_EXTERN_VK_PROC(vkCmdSetStencilTestEnable);
DEFINE_EXTERN_VK_PROC(vkCmdSetStencilWriteMask);
DEFINE_EXTERN_VK_PROC(vkCmdSetViewport);
DEFINE_EXTERN_VK_PROC(vkCmdSetViewportWithCount);
DEFINE_EXTERN_VK_PROC(vkCmdUpdateBuffer);
DEFINE_EXTERN_VK_PROC(vkCmdWaitEvents);
DEFINE_EXTERN_VK_PROC(vkCmdWaitEvents2);
DEFINE_EXTERN_VK_PROC(vkCmdWriteTimestamp);
DEFINE_EXTERN_VK_PROC(vkCmdWriteTimestamp2);
DEFINE_EXTERN_VK_PROC(vkCreateBuffer);
DEFINE_EXTERN_VK_PROC(vkCreateBufferView);
DEFINE_EXTERN_VK_PROC(vkCreateCommandPool);
DEFINE_EXTERN_VK_PROC(vkCreateComputePipelines);
DEFINE_EXTERN_VK_PROC(vkCreateDescriptorPool);
DEFINE_EXTERN_VK_PROC(vkCreateDescriptorSetLayout);
DEFINE_EXTERN_VK_PROC(vkCreateDescriptorUpdateTemplate);
DEFINE_EXTERN_VK_PROC(vkCreateEvent);
DEFINE_EXTERN_VK_PROC(vkCreateFence);
DEFINE_EXTERN_VK_PROC(vkCreateFramebuffer);
DEFINE_EXTERN_VK_PROC(vkCreateGraphicsPipelines);
DEFINE_EXTERN_VK_PROC(vkCreateImage);
DEFINE_EXTERN_VK_PROC(vkCreateImageView);
DEFINE_EXTERN_VK_PROC(vkCreatePipelineCache);
DEFINE_EXTERN_VK_PROC(vkCreatePipelineLayout);
DEFINE_EXTERN_VK_PROC(vkCreatePrivateDataSlot);
DEFINE_EXTERN_VK_PROC(vkCreateQueryPool);
DEFINE_EXTERN_VK_PROC(vkCreateRenderPass);
DEFINE_EXTERN_VK_PROC(vkCreateRenderPass2);
DEFINE_EXTERN_VK_PROC(vkCreateSampler);
DEFINE_EXTERN_VK_PROC(vkCreateSamplerYcbcrConversion);
DEFINE_EXTERN_VK_PROC(vkCreateSemaphore);
DEFINE_EXTERN_VK_PROC(vkCreateShaderModule);
DEFINE_EXTERN_VK_PROC(vkDestroyBuffer);
DEFINE_EXTERN_VK_PROC(vkDestroyBufferView);
DEFINE_EXTERN_VK_PROC(vkDestroyCommandPool);
DEFINE_EXTERN_VK_PROC(vkDestroyDescriptorPool);
DEFINE_EXTERN_VK_PROC(vkDestroyDescriptorSetLayout);
DEFINE_EXTERN_VK_PROC(vkDestroyDescriptorUpdateTemplate);
DEFINE_EXTERN_VK_PROC(vkDestroyEvent);
DEFINE_EXTERN_VK_PROC(vkDestroyFence);
DEFINE_EXTERN_VK_PROC(vkDestroyFramebuffer);
DEFINE_EXTERN_VK_PROC(vkDestroyImage);
DEFINE_EXTERN_VK_PROC(vkDestroyImageView);
DEFINE_EXTERN_VK_PROC(vkDestroyPipeline);
DEFINE_EXTERN_VK_PROC(vkDestroyPipelineCache);
DEFINE_EXTERN_VK_PROC(vkDestroyPipelineLayout);
DEFINE_EXTERN_VK_PROC(vkDestroyPrivateDataSlot);
DEFINE_EXTERN_VK_PROC(vkDestroyQueryPool);
DEFINE_EXTERN_VK_PROC(vkDestroyRenderPass);
DEFINE_EXTERN_VK_PROC(vkDestroySampler);
DEFINE_EXTERN_VK_PROC(vkDestroySamplerYcbcrConversion);
DEFINE_EXTERN_VK_PROC(vkDestroySemaphore);
DEFINE_EXTERN_VK_PROC(vkDestroyShaderModule);
DEFINE_EXTERN_VK_PROC(vkDeviceWaitIdle);
DEFINE_EXTERN_VK_PROC(vkEndCommandBuffer);
DEFINE_EXTERN_VK_PROC(vkFlushMappedMemoryRanges);
DEFINE_EXTERN_VK_PROC(vkFreeCommandBuffers);
DEFINE_EXTERN_VK_PROC(vkFreeDescriptorSets);
DEFINE_EXTERN_VK_PROC(vkFreeMemory);
DEFINE_EXTERN_VK_PROC(vkGetBufferDeviceAddress);
DEFINE_EXTERN_VK_PROC(vkGetBufferMemoryRequirements);
DEFINE_EXTERN_VK_PROC(vkGetBufferMemoryRequirements2);
DEFINE_EXTERN_VK_PROC(vkGetBufferOpaqueCaptureAddress);
DEFINE_EXTERN_VK_PROC(vkGetDescriptorSetLayoutSupport);
DEFINE_EXTERN_VK_PROC(vkGetDeviceBufferMemoryRequirements);
DEFINE_EXTERN_VK_PROC(vkGetDeviceGroupPeerMemoryFeatures);
DEFINE_EXTERN_VK_PROC(vkGetDeviceImageMemoryRequirements);
DEFINE_EXTERN_VK_PROC(vkGetDeviceImageSparseMemoryRequirements);
DEFINE_EXTERN_VK_PROC(vkGetDeviceMemoryCommitment);
DEFINE_EXTERN_VK_PROC(vkGetDeviceMemoryOpaqueCaptureAddress);
DEFINE_EXTERN_VK_PROC(vkGetDeviceQueue);
DEFINE_EXTERN_VK_PROC(vkGetDeviceQueue2);
DEFINE_EXTERN_VK_PROC(vkGetEventStatus);
DEFINE_EXTERN_VK_PROC(vkGetFenceStatus);
DEFINE_EXTERN_VK_PROC(vkGetImageMemoryRequirements);
DEFINE_EXTERN_VK_PROC(vkGetImageMemoryRequirements2);
DEFINE_EXTERN_VK_PROC(vkGetImageSparseMemoryRequirements);
DEFINE_EXTERN_VK_PROC(vkGetImageSparseMemoryRequirements2);
DEFINE_EXTERN_VK_PROC(vkGetImageSubresourceLayout);
DEFINE_EXTERN_VK_PROC(vkGetPipelineCacheData);
DEFINE_EXTERN_VK_PROC(vkGetPrivateData);
DEFINE_EXTERN_VK_PROC(vkGetQueryPoolResults);
DEFINE_EXTERN_VK_PROC(vkGetRenderAreaGranularity);
DEFINE_EXTERN_VK_PROC(vkGetSemaphoreCounterValue);
DEFINE_EXTERN_VK_PROC(vkInvalidateMappedMemoryRanges);
DEFINE_EXTERN_VK_PROC(vkMapMemory);
DEFINE_EXTERN_VK_PROC(vkMergePipelineCaches);
DEFINE_EXTERN_VK_PROC(vkQueueBindSparse);
DEFINE_EXTERN_VK_PROC(vkQueueSubmit);
DEFINE_EXTERN_VK_PROC(vkQueueSubmit2);
DEFINE_EXTERN_VK_PROC(vkQueueWaitIdle);
DEFINE_EXTERN_VK_PROC(vkResetCommandBuffer);
DEFINE_EXTERN_VK_PROC(vkResetCommandPool);
DEFINE_EXTERN_VK_PROC(vkResetDescriptorPool);
DEFINE_EXTERN_VK_PROC(vkResetEvent);
DEFINE_EXTERN_VK_PROC(vkResetFences);
DEFINE_EXTERN_VK_PROC(vkResetQueryPool);
DEFINE_EXTERN_VK_PROC(vkSetEvent);
DEFINE_EXTERN_VK_PROC(vkSetPrivateData);
DEFINE_EXTERN_VK_PROC(vkSignalSemaphore);
DEFINE_EXTERN_VK_PROC(vkTrimCommandPool);
DEFINE_EXTERN_VK_PROC(vkUnmapMemory);
DEFINE_EXTERN_VK_PROC(vkUpdateDescriptorSets);
DEFINE_EXTERN_VK_PROC(vkUpdateDescriptorSetWithTemplate);
DEFINE_EXTERN_VK_PROC(vkWaitForFences);
DEFINE_EXTERN_VK_PROC(vkWaitSemaphores);

/* Device Kronos procs */
DEFINE_EXTERN_VK_PROC(vkAcquireNextImage2KHR);
DEFINE_EXTERN_VK_PROC(vkAcquireNextImageKHR);
DEFINE_EXTERN_VK_PROC(vkCreateSwapchainKHR);
DEFINE_EXTERN_VK_PROC(vkDestroySwapchainKHR);
DEFINE_EXTERN_VK_PROC(vkGetDeviceGroupPresentCapabilitiesKHR);
DEFINE_EXTERN_VK_PROC(vkGetDeviceGroupSurfacePresentModesKHR);
DEFINE_EXTERN_VK_PROC(vkGetImageSubresourceLayout2KHR);
DEFINE_EXTERN_VK_PROC(vkGetSwapchainImagesKHR);
DEFINE_EXTERN_VK_PROC(vkMapMemory2KHR);
DEFINE_EXTERN_VK_PROC(vkQueuePresentKHR);
DEFINE_EXTERN_VK_PROC(vkCmdPushDescriptorSet2KHR);
DEFINE_EXTERN_VK_PROC(vkCmdPushConstants2KHR);

/* Device extension procs */
DEFINE_EXTERN_VK_PROC(vkCmdBindDescriptorBuffersEXT);
DEFINE_EXTERN_VK_PROC(vkCmdBindShadersEXT);
DEFINE_EXTERN_VK_PROC(vkCmdDrawMeshTasksEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetAlphaToCoverageEnableEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetColorBlendEnableEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetColorBlendEquationEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetColorWriteMaskEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetConservativeRasterizationModeEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetDepthClampEnableEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetExtraPrimitiveOverestimationSizeEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetLogicOpEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetLogicOpEnableEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetPolygonModeEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetRasterizationSamplesEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetSampleMaskEXT);
DEFINE_EXTERN_VK_PROC(vkCmdSetVertexInputEXT);
DEFINE_EXTERN_VK_PROC(vkCopyImageToImageEXT);
DEFINE_EXTERN_VK_PROC(vkCopyImageToMemoryEXT);
DEFINE_EXTERN_VK_PROC(vkCopyMemoryToImageEXT);
DEFINE_EXTERN_VK_PROC(vkCreateShadersEXT);
DEFINE_EXTERN_VK_PROC(vkDestroyShaderEXT);
DEFINE_EXTERN_VK_PROC(vkGetDescriptorEXT);
DEFINE_EXTERN_VK_PROC(vkGetDescriptorSetLayoutSizeEXT);
DEFINE_EXTERN_VK_PROC(vkGetImageSubresourceLayout2EXT);
DEFINE_EXTERN_VK_PROC(vkGetMemoryHostPointerPropertiesEXT);
DEFINE_EXTERN_VK_PROC(vkTransitionImageLayoutEXT);

void LoadVkCProcsDevice(VkDevice vk_device);
