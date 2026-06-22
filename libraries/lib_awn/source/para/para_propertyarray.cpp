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

namespace awn::para {

    void PropertyArrayBase::InitializeBgyml(mem::Heap *heap, [[maybe_unused]] Document *document, vp::res::ByamlIterator byaml_iterator) {

        /* Ensure dictionary iterator */
        VP_ASSERT(vp::res::IsContainerType(static_cast<vp::res::ByamlDataType>(byaml_iterator.GetDataType())));

        /* Ensure same size if fixed array type */
        const u32 count = byaml_iterator.GetDataCount();
        VP_ASSERT(m_default->count == cInvalidFixedCount || m_default->count == count);

        /* Create array */
        this->AllocateArray(heap, count);

        /* Read data */
        vp::res::ByamlIterator iter = {};
        for (u32 i = 0; i < count; ++i) {
            const bool result = byaml_iterator.TryGetIteratorByIndex(std::addressof(iter), i);
            if (result) {
                this->SetValueFromByamlIterator(iter, i);
            } else {
                this->SetValueDefault(i);
            }
        }

        return;
    }

#ifdef VP_ENABLE_LIBYAML
    void PropertyArrayBase::InitializeYaml(mem::Heap *heap, [[maybe_unused]] Document *document, vp::res::YamlIterator yaml_iterator) {

        /* Ensure dictionary iterator */
        VP_ASSERT(vp::res::IsContainerType(static_cast<vp::res::ByamlDataType>(yaml_iterator.GetDataType())));

        /* Ensure same size if fixed array type */
        const u32 count = yaml_iterator.GetDataCount();
        VP_ASSERT(m_default->count == cInvalidFixedCount || m_default->count == count);

        /* Create array */
        this->AllocateArray(heap, count);

        /* Read data */
        vp::res::YamlIterator iter = {};
        for (u32 i = 0; i < count; ++i) {
            const bool result = yaml_iterator.TryGetIteratorByIndex(std::addressof(iter), i);
            if (result) {
                this->SetValueFromYamlIterator(iter, i);
            } else {
                this->SetValueDefault(i);
            }
        }

        return;
    }
#endif

    void PropertyArrayBase::InitializeDefaults(mem::Heap *heap, [[maybe_unused]] bool is_force_unlink_parents) {

        /* Handle no default size */
        if (m_default->count == cInvalidFixedCount) { this->AllocateArray(heap, 0); return; }

        /* Allocate and initialize default array */
        this->AllocateArray(heap, m_default->count);
        for (u32 i = 0; i < m_default->count; ++i) {
            this->SetValueDefault(i);
        }

        return;
    }

    void PropertyArrayBase::ResolveParent([[maybe_unused]] Document *document, TypedParam *parent_typed_param, const char *field_name) {

        /* Nothing to do if parenting is disabled */
        if (m_default->is_allow_parenting == 0) { return; }

        /* Unset parent and ensure parent typed param */
        m_parent = nullptr;
        if (parent_typed_param == nullptr) { return; }

        /* Find parent composite */
        FindCompositeContext context = {};
        const Result result = parent_typed_param->ResolveComposite(std::addressof(context), field_name);
        if (result != ResultFoundComposite) { return; }

        /* Set composite if compatible type */
        m_parent = (PropertyArrayBase::CheckRuntimeTypeInfoStatic(context.composite->GetRuntimeTypeInfo()) == true) ? reinterpret_cast<PropertyArrayBase*>(context.composite) : nullptr;

        return;
    }
    
    PropertyArrayBase *PropertyArrayBase::GetParent(u32 index) const {
        PropertyArrayBase *parent = m_parent;
        for (u32 i = index; 0 < i; --i) {
            if (parent == nullptr) { return nullptr; }
            parent = parent->m_parent;
        }
        return parent;
    }

    u32 PropertyArrayBase::GetCount() const {
        const u32 count        = this->GetCountImpl();
        const u32 parent_count = (m_parent != nullptr) ? m_parent->GetCount() : 0;
        return count + parent_count;
    }
}
