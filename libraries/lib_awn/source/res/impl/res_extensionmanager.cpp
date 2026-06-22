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

namespace awn::res::impl {

    Result ExtensionManager::Initialize(mem::Heap *heap, const ExtensionManagerInfo *ext_mgr_info) {

        /* Integrity checks */
        RESULT_RETURN_UNLESS(ext_mgr_info->extension_count < 0xffff'ffff, ResultRangeNG);

        /* Initialize resource unit manager array */
        RESULT_TRY(m_resource_unit_manager_array.Initialize(heap, ext_mgr_info->extension_count + 1));

        /* Set main extension array */
        if (0 < ext_mgr_info->extension_count) {
            RESULT_TRY(m_extension_array.Initialize(heap, ext_mgr_info->extension_count));
            for (u32 i = 0; i < ext_mgr_info->extension_count; ++i) {
                m_extension_array[i] = ext_mgr_info->extension_info_array[i];
            }
            m_extension_array.Sort(ExtensionArray::DefaultCompareLess());
        }

        RESULT_RETURN_SUCCESS;
    }
    void ExtensionManager::Finalize() {
        m_resource_unit_manager_array.Finalize();
        m_extension_array.Finalize();
    }

    bool ExtensionManager::LoopResourceUnitManagers(const ResourceUnitLoopFunction *loop_function) {
        for (ResourceUnitManager &manager : m_resource_unit_manager_array) {
            const bool is_continue = manager.LoopResourceUnits(loop_function);
            if (!is_continue) { return false; }
        }
        return true;
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

        //u32 size  = m_extension_array.GetCount();
        //u32 i     = 0;
        //u32 index = 0;
        //while (i < size) {
        //    index            = i + size;
        //    index            = index >> 1;
        //    const s32 result = ::strcmp(extension, m_extension_array[i].extension.GetString());
        //
        //    if (result == 0) {
        //        return i;
        //    }
        //    if (0 < result) {
        //        i     = index + 1;
        //        index = size;
        //    }
        //    size = index;
        //}
        res::MaxExtensionString ext;
        ext.SetString(extension);
        auto iter = m_extension_array.BinarySearch(ext, ExtensionArray::DefaultCompareLessSearchMember<&ExtensionInfo::extension>());
        return (iter != m_extension_array.end()) ? m_extension_array.IndexOf(std::addressof(*iter)) : cInvalidEntryIndex;
    }
}
