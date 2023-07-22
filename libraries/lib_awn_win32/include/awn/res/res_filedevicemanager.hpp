#pragma once

namespace awn::res {

    class FileDeviceManager {
        private:
            using FileDeviceTree = vp::util::IntrusiveRedBlackTreeTraits<FileDeviceBase, &FileDeviceBase::m_manager_tree_node>::Tree;
        private:
            sys::ServiceCriticalSection  m_device_tree_cs;
            FileDeviceTree               m_mounted_file_device_tree;
            ContentFileDevice           *m_main_file_device;
        public:
            AWN_SINGLETON_TRAITS(FileDeviceManager);
        public:
            constexpr ALWAYS_INLINE FileDeviceManager() : m_device_tree_cs(), m_mounted_file_device_tree(), m_main_file_device() {/*...*/}

            void Initialize(mem::Heap *heap) {

                /* Allocate and initialize ContentFileDevice */
                ContentFileDevice *content_device = new (heap, alignof(ContentFileDevice)) ContentFileDevice();
                VP_ASSERT(content_device != nullptr);

                std::construct_at(content_device);

                m_main_file_device = content_device;

                this->AddFileDevice(content_device);
            }

            void Finalize() {
                if (m_main_file_device != nullptr) {
                    delete m_main_file_device;
                }
                m_main_file_device = nullptr;
            }

            void AddFileDevice(FileDeviceBase *file_device) {
                VP_ASSERT(file_device != nullptr);
                std::scoped_lock l(m_device_tree_cs);
                m_mounted_file_device_tree.Insert(file_device);
            }

            FileDeviceBase *GetFileDeviceByName(const char *device_name) {
                const u32 hash = vp::util::HashCrc32b(device_name);
                std::scoped_lock l(m_device_tree_cs);
                return m_mounted_file_device_tree.Find(hash);
            }

            Result TryLoadFile(FileLoadContext *file_context) {

                /* Get drive */
                MaxDriveString drive;
                vp::util::GetDrive(std::addressof(drive), file_context->file_path);

                /* Find file device */
                FileDeviceBase *device = this->GetFileDeviceByName(drive.GetString());

                /* Get path without drive */
                vp::util::FixedString<vp::util::cMaxPath> path;
                vp::util::GetPathWithoutDrive(std::addressof(path), file_context->file_path);

                /* Load */
                return device->TryLoadFile(file_context);
            }
    };
}
