#pragma once

namespace awn::res {

    class ResourceBase {
        protected:
            void *m_file;
            u32   m_file_size;
        public:
            VP_RTTI_BASE(ResourceBase);
        public:
            constexpr ALWAYS_INLINE ResourceBase() {/*...*/}

            virtual void InitializeResource(mem::Heap *heap, void *file, u32 file_size);

            constexpr ALWAYS_INLINE void *GetFile()           { return m_file; }
            constexpr ALWAYS_INLINE u32   GetFileSize() const { return m_file_size; }
    };
    
    class ArchiveResource : public ResourceBase {
        
    };

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

            virtual ResourceBase *AllocateResource(mem::Heap *heap, s32 alignment) {
                return new (heap, alignment) ResourceBase();
            }
            
            virtual Result TryLoad(ResourceBase *out_resource, ResourceLoadContext *load_arg) {

                /* Allocate resource */
                ResourceBase *res = this->AllocateResource(load_arg->file_load_context.heap, this->GetResourceAlignment());
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
                res->InitializeResource(load_arg->file_load_context.heap, load_arg->file_load_context.out_file, load_arg->file_load_context.out_file_size);

                return ResultSuccess;
            }
            
            constexpr ALWAYS_INLINE virtual u32 GetResourceAlignment() { return alignof(u32); }
    };

    class ResourceFactoryManager {
        public:
            using ResourceFactoryList = vp::util::IntrusiveListTraits<ResourceFactoryBase, &ResourceFactoryBase::m_manager_list_node>::List;
        private:
            sys::ServiceCriticalSection m_list_cs;
            ResourceFactoryList         m_resource_factory_list;
        public:
            AWN_SINGLETON_TRAITS(ResourceFactoryManager);
        public:
            constexpr ALWAYS_INLINE ResourceFactoryManager() {/*...*/}

            void RegisterResourceFactory(ResourceFactoryBase *factory) {
                VP_ASSERT(factory->m_file_extension != nullptr);
                std::scoped_lock lock(m_list_cs);
                m_resource_factory_list.PushBack(*factory);
            }

            void RemoveResourceFactory(ResourceFactoryBase *factory) {
                std::scoped_lock lock(m_list_cs);
                factory->m_manager_list_node.Unlink();
            }

            ResourceFactoryBase *FindResourceFactory(const char *file_extension) {
                std::scoped_lock lock(m_list_cs);
                for (ResourceFactoryBase &rf : m_resource_factory_list) {
                    if (::strcmp(rf.m_file_extension, file_extension) == 0) {
                        return std::addressof(rf);
                    }
                }
                return nullptr;
            }

            void TryLoad(ResourceLoadContext *load_arg) {

                /* Find factory */
                ResourceFactoryBase *factory = load_arg->resource_factory;
                if (factory == nullptr) {
                    vp::util::FixedString<vp::util::cMaxPath> extension;
                    vp::util::GetExtensionFromPath(std::addressof(extension), load_arg->file_load_context.file_path);
                    factory = this->FindResourceFactory(extension.GetString());
                }

                /* Load file with factory */
                RESULT_ABORT_UNLESS(factory->TryLoad(nullptr, load_arg));
            }
    };
}
