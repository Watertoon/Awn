#pragma once

namespace awn::gfx {

	class Texture {
        private:
            VkImage          m_vk_image;
            GpuMemoryAddress m_texture_memory_address;
            TextureInfo      m_texture_info;
        public:
            constexpr ALWAYS_INLINE Texture() : m_vk_image(VK_NULL_HANDLE), m_texture_memory_address() {/*...*/}
            constexpr ALWAYS_INLINE ~Texture() {/*...*/}

            void Initialize(GpuMemoryAddress gpu_memory, TextureInfo *texture_info) {
                VP_ASSERT(texture_info != nullptr);

                m_texture_memory_address = gpu_memory;
                m_vk_image               = m_texture_memory_address.CreateImage(texture_info);
                m_texture_info           = *texture_info;
            }

            void Finalize() {

                /* Delete VkImage */
                if (m_vk_image != VK_NULL_HANDLE) {
                    ::pfn_vkDestroyImage(Context::GetInstance()->GetVkDevice(), m_vk_image, Context::GetInstance()->GetVkAllocationCallbacks());
                }
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
