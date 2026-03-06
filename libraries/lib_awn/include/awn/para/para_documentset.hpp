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

namespace awn::para {

    class DocumentSet;
    class DocumentSetBinder;

    struct DocumentBinderUnit {
        u64 hash;
        DocumentBinderAsync binder;
    };
    struct DocumentSetUnit {
        DocumentSetBinder *set_binder;
        DocumentBinderUnit *binder_unit;
        DocumentSet *set;
    };

    class DocumentSetBinder {
        private:
            impl::DocumentSetUnit *m_unit;
        public:
            constexpr DocumentSetBinder() : m_unit() {/*...*/}
            constexpr ~DocumentSetBinder() {/*...*/}

            bool SetDocumentSet(DocumentSet *set) {
                return set->AllocateSetUnit(this);
            }

            Result TryLoadAsync(const DocumentBinderAsyncInfo *info) {
                RESULT_RETURN_IF(m_unit == nullptr, ResultNoUnit);
                return m_unit->set->AllocateBinderUnit(m_unit, info);
            }
            Result TryLoadAsyncWithKey(u64 key, const DocumentBinderAsyncInfo *info) {
                RESULT_RETURN_IF(m_unit == nullptr, ResultNoUnit);
                return m_unit->set->AllocateBinderUnit(m_unit, key, info);
            }
            void WaitForLoad();
            
    };
    class DocumentSet {
        public:
            struct MapUnit {
                vp::util::IntrusiveRedBlackTreeNode<u64> rb_node;
                impl::DocumentBinderUnit *binder;

                constexpr MapUnit(impl::DocumentBinderUnit *binder) : rb_node(), binder(binder) {/*...*/}
            };
        public:
            using UnitArray = vp::util::FrameArray<DocumentSetUnit>;
            using BinderArray = vp::util::FrameArray<DocumentBinderUnit>;
            using BinderMap = vp::util::IntrusiveRedBlackTreeAllocator<vp::util::IntrusiveRedBlackTreeTraits<MapUnit, &MapUnit::rb_node>>;
        private:
            UnitArray m_unit_array;
            BinderArray m_binder_array;
            BinderMap m_map;
        public:
            constexpr DocumentSet();
            constexpr ~DocumentSet();
            
            constexpr void SetMemory();
        private:
            bool AllocateSetUnit(DocumentSetBinder *binder) {
                const u32 id = m_unit_array.Allocate();
                if (id == IdAllocator::cInvalidEntryIndex) { return false; }
                impl::DocumentSetUnit *unit = std::addressof(m_unit_array[id]);
                binder->m_unit = unit;
                unit->binder = binder;
                return true;
            }
            bool AllocateBinderUnit(impl::DocumentSetUnit *set_unit, const DocumentBinderAsyncInfo *info) {
                const u32 id = m_binder_array.Allocate();
                if (id == IdAllocator::cInvalidEntryIndex) { return false; }
                impl::DocumentBinderUnit *unit = std::addressof(m_binder_array[id]);
                set_unit->binder_unit = unit;
                const bool is_succeed = RESULT_SUCCEEDED(unit->LoadDocumentAsync(info));
                VP_ASSERT(is_succeed);
                return is_succeed;
            }
            bool AllocateBinderUnitWithKey(impl::DocumentSetUnit *set_unit, u64 key, const DocumentBinderAsyncInfo *info) {
                impl::DocumentBinderUnit *ref = this->FindBinderUnit(key);
                if (ref != nullptr) {
                    set_unit->binder_unit = ref;
                    return;
                }
                const u32 id = m_binder_array.Allocate();
                if (id == IdAllocator::cInvalidEntryIndex) { return false; }
                impl::DocumentBinderUnit *unit = std::addressof(m_binder_array[id]);
                set_unit->binder_unit = unit;
                const bool is_succeed = RESULT_SUCCEEDED(unit->LoadDocumentAsync(info));
                VP_ASSERT(is_succeed);
                m_map.Allocate(key, unit);
                return is_succeed;
            }
            impl::DocumentBinderUnit *FindBinderUnit(u64 key) {
                return m_map.Find(key);
            }
    };
}
