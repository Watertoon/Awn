#pragma once

namespace awn::res {

    class ResourceUnitManager;

	struct ExtensionInfo {
        MaxExtensionString  extension;
        CompressionType     compression_type;

        constexpr bool operator<(const ExtensionInfo &rhs) const {
            return extension < rhs.extension;
        }
        constexpr bool operator>(const ExtensionInfo &rhs) const {
            return extension > rhs.extension;
        }
    };

    struct ExtensionManagerInfo {
        u32            extension_count;
        ExtensionInfo *extension_info_array;
    };

    class ExtensionManager {
        public:
            using ExtensionArray           = vp::util::HeapArray<ExtensionInfo>;
            using ResourceUnitManagerArray = vp::util::HeapArray<ResourceUnitManager>;
        private:
            ExtensionArray           m_extension_array;
            ResourceUnitManagerArray m_resource_unit_manager_array;
        public:
            constexpr  ExtensionManager() : m_extension_array(), m_resource_unit_manager_array() {/*...*/}
            constexpr ~ExtensionManager() {/*...*/}

            void Initialize(mem::Heap *heap, const ExtensionManagerInfo *ext_mgr_info);
            void Finalize();

            CompressionType      GetCompressionExtension([[maybe_unused]] const char *group_extension, const char *extension);
            ResourceUnitManager *GetResourceUnitManager([[maybe_unused]] const char *group_extension, const char *extension);

            u32 GetExtensionIndex(const char *extension);

            constexpr inline u32 GetExtensionCount() const {
                return m_extension_array.GetCount();
            }
    };
}
