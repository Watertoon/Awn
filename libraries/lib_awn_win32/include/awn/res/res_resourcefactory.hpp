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
        private:
            friend class ResourceFactoryManager;
        private:
            const char                  *m_file_extension;
            vp::util::IntrusiveListNode  m_manager_list_node;
        public:
            VP_RTTI_BASE(ResourceFactoryBase);
        public:
            constexpr ALWAYS_INLINE explicit ResourceFactoryBase(const char *file_extension) : m_file_extension(file_extension), m_manager_list_node() {/*...*/}
            constexpr virtual ~ResourceFactoryBase() {/*...*/}

            virtual Resource *AllocateResource(mem::Heap *heap, s32 alignment) {
                return new (heap, alignment) Resource();
            }
            
            virtual Result TryLoad(Resource **out_resource, ResourceLoadContext *load_arg) {

                /* Allocate resource */
                Resource *res = this->AllocateResource(load_arg->file_load_context.heap, this->GetResourceAlignment());
                RESULT_RETURN_UNLESS(res != nullptr, ResultResourceAllocationFailed);

                /* Load file */
                Result result = 0;
                if (load_arg->file_device == nullptr) {
                    result = FileDeviceManager::GetInstance()->TryLoadFile(std::addressof(load_arg->file_load_context));
                } else {
                    result = load_arg->file_device->TryLoadFile(std::addressof(load_arg->file_load_context));
                }
                if (result != ResultSuccess) {
                    return result;
                }

                /* Initialize resource */
                res->Initialize(load_arg->file_load_context.heap, load_arg->file_load_context.out_file, load_arg->file_load_context.out_file_size);

                /* Set output */
                *out_resource = res;

                RESULT_RETURN_SUCCESS;
            }

            constexpr ALWAYS_INLINE virtual u32             GetResourceAlignment()         const { return alignof(u32); }
            constexpr ALWAYS_INLINE virtual CompressionType GetPreferrredCompressionType() const { return CompressionType::None; }
    };
}
