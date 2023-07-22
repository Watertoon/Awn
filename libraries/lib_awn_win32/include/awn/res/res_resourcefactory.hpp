#pragma once

namespace awn::res {

    class ResourceFactoryManager;
    class ResourceFactoryBase;

    struct ResourceLoadContext {
        ResourceFactoryBase *resource_factory;
        FileDeviceBase      *file_device;
        FileLoadContext      file_load_context;
    };

    class ResourceFactoryBase {
        public:
            static constexpr u32             cResourceSize             = sizeof(Resource);
            static constexpr u32             cResourceAlignment        = alignof(u32);
            static constexpr CompressionType cPreferredCompressionType = CompressionType::None;
            static constexpr u32             cDefaultFileSizeScale     = 1.0f;
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
            constexpr virtual ~ResourceFactoryBase() {/*...*/}

            virtual Resource *AllocateResource(mem::Heap *heap, s32 alignment) {
                return new (heap, alignment) Resource();
            }

            virtual Result TryLoadWithDecompressor([[maybe_unused]] Resource **out_resource, [[maybe_unused]] ResourceLoadContext *load_arg, [[maybe_unused]] DecompressorBase *decompressor) {

            //    /* Stream in file  */
            //    decompressor->TryStreamByDevice(load_arg);
            //
            //    /**/
            //    Resource *resource = this->AllocateResource();
            //
            //
            //    *out_resource = resource;

                RESULT_RETURN_SUCCESS;
            }

            virtual Result TryLoad(Resource **out_resource, ResourceLoadContext *load_arg) {

                /* Allocate resource */
                Resource *resource = this->AllocateResource(load_arg->file_load_context.heap, this->GetResourceAlignment());
                RESULT_RETURN_UNLESS(resource != nullptr, ResultResourceAllocationFailed);

                /* Load file */
                Result result = 0;
                if (load_arg->file_device == nullptr) {
                    result = FileDeviceManager::GetInstance()->TryLoadFile(std::addressof(load_arg->file_load_context));
                } else {
                    result = load_arg->file_device->TryLoadFile(std::addressof(load_arg->file_load_context));
                }
                if (result != ResultSuccess) {
                    ::operator delete (resource, load_arg->file_load_context.heap, this->GetResourceAlignment());
                    return result;
                }

                /* Initialize resource */
                resource->Initialize(load_arg->file_load_context.heap, load_arg->file_load_context.out_file, load_arg->file_load_context.out_file_size);

                /* Set output */
                *out_resource = resource;

                RESULT_RETURN_SUCCESS;
            }

            constexpr virtual u32             GetResourceSize()             const { return cResourceSize; }
            constexpr virtual u32             GetResourceAlignment()        const { return cResourceAlignment; }
            constexpr virtual CompressionType GetPreferredCompressionType() const { return cPreferredCompressionType; }
            constexpr virtual float           GetDefaultFileSizeScale()     const { return cDefaultFileSizeScale; }
            constexpr virtual u32             GetDefaultExtraResourceSize() const { return cDefaultExtraSize; }
    };
}
