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

namespace awn::gfx {

	class Texture {
        private:
            VkImage          m_vk_image;
            TextureInfo      m_texture_info;
        public:
            constexpr ALWAYS_INLINE Texture() : m_vk_image(VK_NULL_HANDLE), m_texture_info() {/*...*/}
            constexpr ALWAYS_INLINE ~Texture() {/*...*/}

            void Initialize(void *gpu_memory_address, TextureInfo *texture_info) {

                /* Integrity check */
                VP_ASSERT(texture_info != nullptr);

                /* Create image by gpu memory address */
                m_vk_image     = gfx::CreateVkImage(gpu_memory_address, texture_info);
                m_texture_info = *texture_info;

                return;
            }

            void Initialize(VkDeviceMemory vk_device_memory, size_t memory_offset, TextureInfo *texture_info) {

                /* Integrity checks */
                VP_ASSERT(vk_device_memory != VK_NULL_HANDLE);
                VP_ASSERT(texture_info != nullptr);

                /* Create VkImage */
                VkImage image = 0;
                const VkImageCreateInfo image_info = {
                    .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .flags                 = vp::res::ResGfxTextureInfoToVkImageCreateFlags(texture_info),
                    .imageType             = vp::res::GfxImageStorageDimensionToVkImageType(static_cast<ImageStorageDimension>(texture_info->storage_dimension)),
                    .format                = vp::res::GfxImageFormatToVkFormat(static_cast<ImageFormat>(texture_info->image_format)),
                    .extent = {
                        .width             = texture_info->width,
                        .height            = texture_info->height,
                        .depth             = texture_info->depth
                    },
                    .mipLevels             = texture_info->mip_levels,
                    .arrayLayers           = texture_info->array_layer_count,
                    .samples               = static_cast<VkSampleCountFlagBits>(texture_info->sample_count),
                    .tiling                = static_cast<VkImageTiling>(texture_info->tile_mode),
                    .usage                 = vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags)),
                    .sharingMode           = VK_SHARING_MODE_CONCURRENT,
                    .queueFamilyIndexCount = Context::GetInstance()->GetQueueFamilyCount(),
                    .pQueueFamilyIndices   = Context::GetInstance()->GetQueueFamilyIndiceArray(),
                    .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
                };
                const u32 result0 = ::pfn_vkCreateImage(Context::GetInstance()->GetVkDevice(), std::addressof(image_info), Context::GetInstance()->GetVkAllocationCallbacks(), std::addressof(image));
                VP_ASSERT(result0 == VK_SUCCESS);

                /* Bind gpu memory to image from offset */
                const u32 result1 = ::pfn_vkBindImageMemory(Context::GetInstance()->GetVkDevice(), image, vk_device_memory, memory_offset);
                VP_ASSERT(result1 == VK_SUCCESS);

                return;
            }

            void Finalize() {

                /* Delete VkImage */
                if (m_vk_image != VK_NULL_HANDLE) {
                    ::pfn_vkDestroyImage(Context::GetInstance()->GetVkDevice(), m_vk_image, Context::GetInstance()->GetVkAllocationCallbacks());
                }
            }

            static GpuMemoryRequirements GetMemoryRequirements(TextureInfo *texture_info) {

                /* Get memory requirements */
                const VkImageCreateInfo image_info = {
                    .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .flags                 = vp::res::ResGfxTextureInfoToVkImageCreateFlags(texture_info),
                    .imageType             = vp::res::GfxImageStorageDimensionToVkImageType(static_cast<ImageStorageDimension>(texture_info->storage_dimension)),
                    .format                = vp::res::GfxImageFormatToVkFormat(static_cast<ImageFormat>(texture_info->image_format)),
                    .extent = {
                        .width             = texture_info->width,
                        .height            = texture_info->height,
                        .depth             = texture_info->depth
                    },
                    .mipLevels             = texture_info->mip_levels,
                    .arrayLayers           = texture_info->array_layer_count,
                    .samples               = static_cast<VkSampleCountFlagBits>(texture_info->sample_count),
                    .tiling                = static_cast<VkImageTiling>(texture_info->tile_mode),
                    .usage                 = vp::res::GfxGpuAccessFlagsToVkImageUsageFlags(static_cast<vp::res::GfxGpuAccessFlags>(texture_info->gpu_access_flags)),
                    .sharingMode           = VK_SHARING_MODE_CONCURRENT,
                    .queueFamilyIndexCount = Context::GetInstance()->GetQueueFamilyCount(),
                    .pQueueFamilyIndices   = Context::GetInstance()->GetQueueFamilyIndiceArray(),
                    .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
                };
                const VkDeviceImageMemoryRequirements memory_requirement_info = {
                    .sType       = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS,
                    .pCreateInfo = std::addressof(image_info),
                };
                VkMemoryRequirements2 memory_requirements_2 = {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
                };
                ::pfn_vkGetDeviceImageMemoryRequirements(Context::GetInstance()->GetVkDevice(), std::addressof(memory_requirement_info), std::addressof(memory_requirements_2));

                return { memory_requirements_2.memoryRequirements.size, memory_requirements_2.memoryRequirements.alignment };
            }

            constexpr ALWAYS_INLINE u32 GetWidth() const {
                return m_texture_info.width;
            }
            constexpr ALWAYS_INLINE u32 GetHeight() const {
                return m_texture_info.height;
            }
            constexpr ALWAYS_INLINE ImageFormat GetImageFormat() const {
                return static_cast<ImageFormat>(m_texture_info.image_format);
            }
            constexpr ALWAYS_INLINE VkImage GetVkImage() const {
                return m_vk_image;
            }
    };
    static_assert(sizeof(Texture) <= cMaxGfxTextureSize);
}
