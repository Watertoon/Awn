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

namespace awn::res {

    namespace impl {        
        class ResourceFactoryManager;
    }
    class ResourceFactoryBase;

    struct ResourceLoadContext {
        FileLoadContext            file_load_context;
        FileDeviceBase            *file_device;
        const ResourceFactoryBase *resource_factory;
        mem::Heap                 *resource_heap;
        mem::Heap                 *gpu_heap;
        s32                        resource_alignment;
    };

    class ResourceFactoryBase {
        public:
            static constexpr u32             cResourceSize             = sizeof(Resource);
            static constexpr u32             cResourceAlignment        = alignof(Resource);
            static constexpr u32             cFileAlignment            = alignof(u32);
            static constexpr CompressionType cDefaultCompressionType   = CompressionType::None;
            static constexpr float           cDefaultFallbackSizeScale = 1.0f;
            static constexpr u32             cDefaultExtraSize         = 0;
        private:
            friend class impl::ResourceFactoryManager;
        private:
            vp::util::IntrusiveRedBlackTreeNode<u32>  m_manager_tree_node;
            const char                               *m_file_extension;
            u32                                       m_resource_size;
            u32                                       m_resource_alignment;
            u32                                       m_file_alignment;
            CompressionType                           m_default_compression_type;
            float                                     m_default_fallback_size_scale;
            u32                                       m_default_extra_size;
            bool                                      m_is_initialize_on_create;
            bool                                      m_is_boost_compose;
        public:
            VP_RTTI_BASE(ResourceFactoryBase);
        public:
            constexpr ALWAYS_INLINE ResourceFactoryBase() : m_manager_tree_node(), m_file_extension(), m_resource_size(cResourceSize), m_resource_alignment(cResourceAlignment), m_file_alignment(cFileAlignment), m_default_compression_type(cDefaultCompressionType), m_default_fallback_size_scale(cDefaultFallbackSizeScale), m_default_extra_size(cDefaultExtraSize), m_is_initialize_on_create(true), m_is_boost_compose() {/*...*/}
            constexpr ALWAYS_INLINE ResourceFactoryBase(const char *file_extension, u32 file_alignment = cFileAlignment) : m_manager_tree_node(), m_file_extension(file_extension), m_resource_size(cResourceSize), m_resource_alignment(cResourceAlignment), m_file_alignment(file_alignment), m_default_compression_type(cDefaultCompressionType), m_default_fallback_size_scale(cDefaultFallbackSizeScale), m_default_extra_size(cDefaultExtraSize), m_is_initialize_on_create(true), m_is_boost_compose() {
                const u32 hash_crc32 = vp::util::HashCrc32b(file_extension);
                m_manager_tree_node.SetKey(hash_crc32);
            }
            constexpr ALWAYS_INLINE ResourceFactoryBase(const char *file_extension, u32 resource_size, u32 resource_alignment, u32 file_alignment, CompressionType default_compression_type, float fallback_size_scale, u32 fallback_extra_size, bool is_init_on_create, bool is_boost_compose) : m_manager_tree_node(), m_file_extension(file_extension), m_resource_size(resource_size), m_resource_alignment(resource_alignment), m_file_alignment(file_alignment), m_default_compression_type(default_compression_type), m_default_fallback_size_scale(fallback_size_scale), m_default_extra_size(fallback_extra_size), m_is_initialize_on_create(is_init_on_create), m_is_boost_compose(is_boost_compose) {
                const u32 hash_crc32 = vp::util::HashCrc32b(file_extension);
                m_manager_tree_node.SetKey(hash_crc32);
            }
            virtual ~ResourceFactoryBase();

            Result LoadResourceWithDecompressor(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context, IDecompressor *decompressor) const;
            Result LoadResource(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context) const;
        public:
            virtual Resource *AllocateResource(mem::Heap *heap, s32 alignment) const {
                return new (heap, alignment) Resource();
            }

            constexpr bool            IsInitializeOnCreate()         const { return m_is_initialize_on_create; }
            constexpr bool            IsBoostCompose()               const { return m_is_boost_compose; }
            constexpr u32             GetResourceSize()              const { return m_resource_size; }
            constexpr u32             GetResourceAlignment()         const { return m_resource_alignment; }
            constexpr u32             GetFileAlignment()             const { return m_file_alignment; }
            constexpr CompressionType GetDefaultCompressionType()    const { return m_default_compression_type; }
            constexpr float           GetDefaultFallbackSizeScale()  const { return m_default_fallback_size_scale; }
            constexpr u32             GetDefaultExtraResourceSize()  const { return m_default_extra_size; }

    };
}
