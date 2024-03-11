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
#include <awn.hpp>

namespace awn::res {

    void ExtensionManager::Initialize(mem::Heap *heap, const ExtensionManagerInfo *ext_mgr_info) {

        /* Integrity checks */
        VP_ASSERT(ext_mgr_info != nullptr);

        /* Initialize resource unit manager array */
        m_resource_unit_manager_array.Initialize(heap, ext_mgr_info->extension_count + 1);

        /* Set main extension array */
        if (ext_mgr_info->extension_count < 1) { return; }
        m_extension_array.Initialize(heap, ext_mgr_info->extension_count);
        for (u32 i = 0; i < ext_mgr_info->extension_count; ++i) {
            m_extension_array[i] = ext_mgr_info->extension_info_array[i];
        }
        m_extension_array.Sort();

        return;
    }
    void ExtensionManager::Finalize() {
        m_resource_unit_manager_array.Finalize();
        m_extension_array.Finalize();
    }

    CompressionType ExtensionManager::GetCompressionExtension([[maybe_unused]] const char *group_extension, const char *extension) {
        const u32 index = this->GetExtensionIndex(extension);
        return (index != cInvalidEntryIndex) ? m_extension_array[index].compression_type : CompressionType::Auto;
    }

    ResourceUnitManager *ExtensionManager::GetResourceUnitManager([[maybe_unused]] const char *group_extension, const char *extension) {
        const u32 index = this->GetExtensionIndex(extension);
        return (index != cInvalidEntryIndex) ? std::addressof(m_resource_unit_manager_array[index + 1]) : std::addressof(m_resource_unit_manager_array[0]);
    }

    u32 ExtensionManager::GetExtensionIndex(const char *extension) {

        u32 size  = m_extension_array.GetCount();
        u32 i     = 0;
        u32 index = 0;
        while (i < size) {
            index            = i + size;
            index            = index >> 1;
            const s32 result = ::strcmp(extension, m_extension_array[i].extension.GetString());

            if (result == 0) {
                return i;
            }
            if (0 < result) {
                i     = index + 1;
                index = size;
            }
            size = index;
        }

        return cInvalidEntryIndex;
    }
}
