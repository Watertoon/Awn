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

    class ResourceFactoryManager;
    class ResourceFactoryBase;

    struct ResourceLoadContext : public FileLoadContext {
        FileDeviceBase      *file_device;
        ResourceFactoryBase *resource_factory;
        ResourceUserInfo    *resource_user_info;
        mem::Heap           *resource_heap;
        s32                  resource_alignment;
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
            friend class ResourceFactoryManager;
        private:
            vp::util::IntrusiveRedBlackTreeNode<u32>  m_manager_tree_node;
            const char                               *m_file_extension;
        public:
            VP_RTTI_BASE(ResourceFactoryBase);
        public:
            explicit constexpr ALWAYS_INLINE ResourceFactoryBase(const char *file_extension) : m_manager_tree_node(), m_file_extension(file_extension) {
                const u32 hash_crc32 = vp::util::HashCrc32b(file_extension);
                m_manager_tree_node.SetKey(hash_crc32);
            }
            virtual constexpr ~ResourceFactoryBase() {/*...*/}

            virtual Resource *AllocateResource(mem::Heap *heap, s32 alignment) {
                return new (heap, alignment) Resource();
            }

            virtual constexpr u32             GetResourceSize()             const { return cResourceSize; }
            virtual constexpr u32             GetResourceAlignment()        const { return cResourceAlignment; }
            virtual constexpr u32             GetFileAlignment()            const { return cFileAlignment; }
            virtual constexpr CompressionType GetDefaultCompressionType()   const { return cDefaultCompressionType; }
            virtual constexpr float           GetDefaultFallbackSizeScale() const { return cDefaultFallbackSizeScale; }
            virtual constexpr u32             GetDefaultExtraResourceSize() const { return cDefaultExtraSize; }

            Result LoadResourceWithDecompressor(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context, IDecompressor *decompressor) {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(out_resource != nullptr, ResultNullOutputResource);

                /* Allocate resource */
                Resource *resource = this->AllocateResource(resource_load_context->resource_heap, this->GetResourceAlignment());
                RESULT_RETURN_UNLESS(resource != nullptr,     ResultResourceAllocationFailed);
                RESULT_RETURN_UNLESS(decompressor != nullptr, ResultInvalidDecompressor);

                /* Load decompressed */
                const Result result = decompressor->TryLoadDecompress(nullptr, nullptr, path, resource_load_context);
                if (result != ResultSuccess) {
                    delete resource;
                    return result;
                }

                /* Setup resource */
                resource->m_file      = resource_load_context->file_buffer;
                resource->m_file_size = resource_load_context->file_size;
                resource->m_user_info = resource_load_context->resource_user_info;

                /* Resource pre-prepare */
                resource->OnFileLoad(resource_load_context->resource_heap, resource_load_context->file_buffer, resource_load_context->file_size);

                /* Set output */
                *out_resource = resource;

                RESULT_RETURN_SUCCESS;
            }

            Result LoadResource(Resource **out_resource, const char *path, ResourceLoadContext *resource_load_context) {

                /* Integrity checks */
                RESULT_RETURN_UNLESS(out_resource != nullptr, ResultNullOutputResource);

                /* Allocate resource */
                Resource *resource = this->AllocateResource(resource_load_context->resource_heap, this->GetResourceAlignment());
                RESULT_RETURN_UNLESS(resource != nullptr, ResultResourceAllocationFailed);

                /* Load file */
                Result result = 0;
                if (resource_load_context->file_device == nullptr) {
                    result = FileDeviceManager::GetInstance()->LoadFile(path, resource_load_context);
                } else {
                    result = resource_load_context->file_device->LoadFile(path, resource_load_context);
                }
                if (result != ResultSuccess) {
                    delete resource;
                    return result;
                }

                /* Setup resource */
                resource->m_file      = resource_load_context->file_buffer;
                resource->m_file_size = resource_load_context->file_size;
                resource->m_user_info = resource_load_context->resource_user_info;

                /* Resource load callback */
                resource->OnFileLoad(resource_load_context->resource_heap, resource_load_context->file_buffer, resource_load_context->file_size);

                /* Set output */
                *out_resource = resource;

                RESULT_RETURN_SUCCESS;
            }

    };
}
