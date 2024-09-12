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

    struct PropertyArrayDefaultBase {
        u32 is_allow_parenting;
        u32 count;
    };

    template <typename T>
    struct PropertyArrayDefault {
        T   value;
    };

    class PropertyArrayBase : public Composite {
        public:
            static constexpr u32 cInvalidFixedCount = 0xffff'ffff;
        public:
            PropertyArrayDefaultBase *m_default;
            PropertyArrayBase        *m_parent;
        public:
            VP_RTTI_DERIVED(PropertyArrayBase, Composite);
        public:
            constexpr PropertyArrayBase() : Composite(), m_default(), m_parent() {/*...*/}
            virtual constexpr ~PropertyArrayBase() override {/*...*/}

            virtual void InitializeBgyml(mem::Heap *heap, Document *document, vp::res::ByamlIterator byaml_iterator) override;
            virtual void InitializeDefaults(mem::Heap *heap, bool is_force_unlink_parents) override;

            virtual void ResolveParent(Document *document, TypedParam *parent_typed_param, const char *field_name) override;

            virtual void AllocateArray(mem::Heap *heap, u32 count) = 0;
            virtual void SetValueFromByamlData(vp::res::ByamlIterator iter, vp::res::ByamlData data, u32 i) = 0;
            virtual void SetValueDefault(u32 i) = 0;
            virtual constexpr u32 GetCountImpl() const = 0;
        public:
            PropertyArrayBase *GetParent(u32 index) const;
            u32 GetCount() const;
    };
    
    template <typename T>
    class PropertyArray : public PropertyArrayBase {
        public:
            class Iterator {
                private:
                    T *m_element;
                public:
                    constexpr Iterator(T *element) : m_element(element) {/*...*/}
                    constexpr ~Iterator() {/*...*/}

                    constexpr T operator*() const {
                        return *m_element;
                    }

                    constexpr Iterator &operator++() {
                        m_element = m_element + 1;
                        return *this;
                    }

                    constexpr bool operator!=(const Iterator &rhs) {
                        return m_element != rhs.m_element;
                    }
            };
        private:
            T   *m_array;
            u32  m_max;
            u32  m_count;
        public:
            VP_RTTI_DERIVED(PropertyArray, PropertyArrayBase);
        public:
            constexpr PropertyArray() : PropertyArrayBase(), m_array(), m_max(), m_count() {/*...*/}
            virtual constexpr ~PropertyArray() override {/*...*/}

            constexpr Iterator begin() {
                return Iterator(m_array);
            }
            constexpr Iterator end() {
                return Iterator(m_array + m_count);
            }

            virtual void Finalize() override {
                if (m_array != nullptr) {
                    delete m_array;
                    m_array = nullptr;
                    m_max   = 0;
                    m_count = 0;
                }
            }

            virtual void AllocateArray(mem::Heap *heap, u32 count) override {

                /* Check if array is already allocated */
                if (m_array != nullptr) { VP_ASSERT(count < m_max); m_count = count; return; }

                /* Ensure not zero */
                if (count < 1) { return; }

                /* Allocate array */
                m_array = new (heap, alignof(T)) T[count];
                m_max   = count;
                m_count = count;

                return;
            }

            virtual void SetValueFromByamlData(vp::res::ByamlIterator iter, vp::res::ByamlData data, u32 i) override {

                /* Ensure index */
                VP_ASSERT(i < m_count);

                /* Get array offset */
                T *array_offset = m_array + i;

                /* Try load data */
                const bool result = iter.TryGetValueByData(array_offset, data);

                /* Set default */
                if (result == false) {
                    *array_offset = reinterpret_cast<PropertyArrayDefault<T>*>(m_default)->value;
                }

                return;
            }

            virtual void SetValueDefault(u32 i) override {

                /* Ensure index */
                VP_ASSERT(i < m_count);

                /* Set default */
                T *array_offset = m_array + i;
                *array_offset   = reinterpret_cast<PropertyArrayDefault<T>*>(m_default)->value;

                return;
            }

            virtual constexpr u32 GetCountImpl() const { return m_count; }
    };
}
