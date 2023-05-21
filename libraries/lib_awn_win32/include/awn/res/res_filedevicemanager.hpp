#pragma once

namespace awn::res {

    class FileDeviceManager {
        private:
            using FileDeviceList = vp::util::IntrusiveListTraits<FileDeviceBase, &FileDeviceBase::m_manager_list_node>::List;
        private:
            FileDeviceList     m_mounted_file_device_list;
            ContentFileDevice *m_main_file_device;
        public:
            AWN_SINGLETON_TRAITS(FileDeviceManager);
        public:
            constexpr ALWAYS_INLINE FileDeviceManager() {/*...*/}

            void Initialize(mem::Heap *heap) {

                /* Allocate and initialize ContentFileDevice */
                ContentFileDevice *content_device = new (heap, alignof(ContentFileDevice)) ContentFileDevice();
                VP_ASSERT(content_device != nullptr);

                std::construct_at(content_device);

                m_main_file_device = content_device;
            }

            void AddFileDevice(FileDeviceBase *file_device) {
                VP_ASSERT(file_device != nullptr);
                m_mounted_file_device_list.PushBack(*file_device);
            }

            FileDeviceBase *GetFileDeviceByName(const char *device_name) {

                /* Lookup device by name */
                for (FileDeviceBase &device : m_mounted_file_device_list) {
                    const u32 cmp_result = ::strncmp(device.GetDeviceName(), device_name, vp::util::cMaxDrive);
                    if (cmp_result == 0) { return std::addressof(device); }
                }
                return nullptr;
            }

            Result TryLoadFile(FileLoadContext *file_context) {

                /* Get drive */
                vp::util::FixedString<vp::util::cMaxDrive> drive;
                vp::util::GetDriveFromPath(std::addressof(drive), file_context->file_path);

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
