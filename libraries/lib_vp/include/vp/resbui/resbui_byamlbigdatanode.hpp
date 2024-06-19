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

namespace vp::resbui {

    class ByamlNodeBase;

	class ByamlBigDataNodeBase {
        public:
			static constexpr u32 cInvalidOffset = 0xffff'ffff;
		protected:
			u32 m_offset;
		public:
            constexpr  ByamlBigDataNodeBase() : m_offset(cInvalidOffset) {/*...*/}
            virtual ~ByamlBigDataNodeBase() {/*...*/}

            virtual void PushKeys([[maybe_unused]] ByamlStringPoolBuilder *pool_builder) {/*...*/}
            virtual void PushStrings([[maybe_unused]] ByamlStringPoolBuilder *pool_builder) {/*...*/}

            virtual constexpr res::ByamlDataType GetByamlDataType() const { return res::ByamlDataType::Null; }
            virtual void Serialize([[maybe_unused]] uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, [[maybe_unused]] vp::res::ResByaml *head) {/*...*/}

            virtual void CalculateEndOffset([[maybe_unused]] uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t *container_offset_iter, [[maybe_unused]] uintptr_t head) { return; }

            constexpr u32 GetOffset() const { return m_offset; }
	};

    /* ByamlNodeBigData definitions */
    constexpr res::ByamlDataType ByamlNodeBigData::GetByamlDataType() const { return m_big_data_node->GetByamlDataType(); }

    class ByamlBigDataNodeContainerBase : public ByamlBigDataNodeBase {
        public:
            using NodeList = vp::util::IntrusiveListTraits<ByamlNodeBase, &ByamlNodeBase::m_container_node>::List;
        protected:
            u32      m_node_count;
            NodeList m_node_list;
        public:
            constexpr  ByamlBigDataNodeContainerBase() : ByamlBigDataNodeBase(), m_node_count(), m_node_list() {/*...*/}
            virtual ~ByamlBigDataNodeContainerBase() override { this->Finalize(); }
    
            void Finalize();

            virtual void PushKeys(ByamlStringPoolBuilder *pool_builder) override;
            virtual void PushStrings([[maybe_unused]] ByamlStringPoolBuilder *pool_builder) override;
    };

    enum class ByamlArrayDataTypeMode : u8 {
        Auto       = 0,
        MonoTyped  = 1,
        MultiTyped = 2,
    };
    class ByamlBigDataNodeArray : public ByamlBigDataNodeContainerBase {
        protected:
            ByamlArrayDataTypeMode m_data_type_mode;
        private:
            constexpr bool CheckMonoTyped() const {
                if (m_data_type_mode != ByamlArrayDataTypeMode::Auto) { return (m_data_type_mode == ByamlArrayDataTypeMode::MonoTyped); }
                const res::ByamlDataType ref_data_type = (m_node_list.IsEmpty() == false) ? m_node_list.Front().GetByamlDataType() : res::ByamlDataType::Null;
                for (ByamlNodeBase &node : m_node_list) {
                    if (node.GetByamlDataType() != ref_data_type) { return false; }
                }
                return true;
            }
        public:
            constexpr ByamlBigDataNodeArray() : ByamlBigDataNodeContainerBase(), m_data_type_mode(ByamlArrayDataTypeMode::Auto) {/*...*/}
            virtual ~ByamlBigDataNodeArray() override {/*...*/}

            constexpr void SetArrayMode(ByamlArrayDataTypeMode mode) { m_data_type_mode = mode; }

            void AddNode(ByamlNodeBase *node);
            void RemoveNode(ByamlNodeBase *node);

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return (this->CheckMonoTyped() == true) ? res::ByamlDataType::MonoTypedArray : res::ByamlDataType::Array; }

            virtual void Serialize(uintptr_t *big_data_offset, uintptr_t *container_offset, vp::res::ResByaml *head) override;

            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, uintptr_t *container_offset_iter, uintptr_t head) override;
    };

    class ByamlBigDataNodeDictionary : public ByamlBigDataNodeContainerBase {
        private:
            bool m_is_index_remap;
        private:
            constexpr u32 CalculateRemapSize() const {

                if (m_is_index_remap == false) { return 0; }
                if (m_node_count < 0x100)      { return sizeof(u8) * m_node_count; }
                if (m_node_count < 0x10000)    { return sizeof(u16) * m_node_count; }
                return sizeof(u32) * m_node_count;
            }
        public:
            constexpr ByamlBigDataNodeDictionary() : ByamlBigDataNodeContainerBase(), m_is_index_remap(false) {/*...*/}
            virtual ~ByamlBigDataNodeDictionary() override {/*...*/}

            bool AddNode(ByamlNodeBase *node, const char *key, u32 index = ByamlNodeBase::cInvalidIndex);
            void RemoveNode(ByamlNodeBase *node);

            constexpr void SetIsIndexRemap(bool is_index_remap) { m_is_index_remap = is_index_remap; }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return (m_is_index_remap == true) ? res::ByamlDataType::DictionaryWithRemap : res::ByamlDataType::Dictionary; }

            virtual void Serialize(uintptr_t *big_data_offset, uintptr_t *container_offset, vp::res::ResByaml *head) override;

            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, uintptr_t *container_offset_iter, uintptr_t head) override;
    };

    class ByamlBigDataNodeHashArray : public ByamlBigDataNodeContainerBase {
        private:
            bool m_is_index_remap;
            u32  m_key_size;
        private:
            constexpr u32 CalculateRemapSize() const {

                if (m_is_index_remap == false) { return 0; }
                if (m_node_count < 0x100)      { return sizeof(u8) * m_node_count; }
                if (m_node_count < 0x10000)    { return sizeof(u16) * m_node_count; }
                return sizeof(u32) * m_node_count;
            }
        public:
            constexpr ByamlBigDataNodeHashArray() : ByamlBigDataNodeContainerBase(), m_is_index_remap(false), m_key_size(sizeof(u32)) {/*...*/}
            virtual ~ByamlBigDataNodeHashArray() override {/*...*/}

            bool AddNode(ByamlNodeBase *node, u64 hash, u32 index = ByamlNodeBase::cInvalidIndex);

            void RemoveNode(ByamlNodeBase *node);

            constexpr void SetIsIndexRemap(bool is_index_remap) { m_is_index_remap = is_index_remap; }
            constexpr void SetKeySize(u32 key_size) { m_key_size = key_size; }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { 
                const u32 sub_type = ((m_key_size / sizeof(u32)) - 1);
                return (m_is_index_remap == true) ? static_cast<res::ByamlDataType>(static_cast<u8>(res::ByamlDataType::HashArrayWithRemapU32_1) | sub_type) : static_cast<res::ByamlDataType>(static_cast<u8>(res::ByamlDataType::HashArrayU32_1) | sub_type); 
            }

            virtual void Serialize(uintptr_t *big_data_offset, uintptr_t *container_offset, vp::res::ResByaml *head) override;

            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, uintptr_t *container_offset_iter, uintptr_t head) override;
    };

    class ByamlBigDataNodeU64 : public ByamlBigDataNodeBase {
        private:
            u64 m_value;
        public:
            constexpr ByamlBigDataNodeU64() : ByamlBigDataNodeBase(), m_value() {/*...*/}
            constexpr ByamlBigDataNodeU64(u64 value) : ByamlBigDataNodeBase(), m_value(value) {/*...*/}
            virtual ~ByamlBigDataNodeU64() override {/*...*/}

            constexpr void SetValue(u64 value) { m_value = value; }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return res::ByamlDataType::U64; }

            virtual void Serialize(uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, [[maybe_unused]] vp::res::ResByaml *head) override;

            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t *container_offset_iter, [[maybe_unused]] uintptr_t head) override;
    };

    class ByamlBigDataNodeS64 : public ByamlBigDataNodeBase {
        private:
            s64 m_value;
        public:
            constexpr ByamlBigDataNodeS64() : ByamlBigDataNodeBase(), m_value() {/*...*/}
            constexpr ByamlBigDataNodeS64(s64 value) : ByamlBigDataNodeBase(), m_value(value) {/*...*/}
            virtual ~ByamlBigDataNodeS64() override {/*...*/}

            constexpr void SetValue(s64 value) { m_value = value; }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return res::ByamlDataType::S64; }

            virtual void Serialize(uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, [[maybe_unused]] vp::res::ResByaml *head) override;

            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t *container_offset_iter, [[maybe_unused]] uintptr_t head) override;
    };

    class ByamlBigDataNodeF64 : public ByamlBigDataNodeBase {
        private:
            double m_value;
        public:
            constexpr ByamlBigDataNodeF64() : ByamlBigDataNodeBase(), m_value() {/*...*/}
            constexpr ByamlBigDataNodeF64(double value) : ByamlBigDataNodeBase(), m_value(value) {/*...*/}
            virtual ~ByamlBigDataNodeF64() override {/*...*/}

            constexpr void SetValue(double value) { m_value = value; }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return res::ByamlDataType::F64; }

            virtual void Serialize(uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, vp::res::ResByaml *head) override;

            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t *container_offset_iter, [[maybe_unused]] uintptr_t head) override;
    };

    class ByamlBigDataNodeBinary : public ByamlBigDataNodeBase {
        public:
            static constexpr u32 cInvalidAlignment = 0xffff'ffff;
        private:
            void *m_data;
            u32   m_data_size;
            u32   m_data_alignment;
        public:
            constexpr ByamlBigDataNodeBinary() : ByamlBigDataNodeBase(), m_data(), m_data_size(0), m_data_alignment(cInvalidAlignment) {/*...*/}
            virtual ~ByamlBigDataNodeBinary() override {/*...*/}

            constexpr void SetBinaryData(void *data, u32 size, u32 alignment = cInvalidAlignment) {
                m_data           = data;
                m_data_size      = size;
                m_data_alignment = alignment;
            }

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return (m_data_alignment == cInvalidAlignment) ? res::ByamlDataType::BinaryData : res::ByamlDataType::BinaryDataWithAlignment; }

            virtual void Serialize(uintptr_t *big_data_offset, [[maybe_unused]] uintptr_t *container_offset, vp::res::ResByaml *head) override;

            virtual void CalculateEndOffset(uintptr_t *big_data_offset_iter, [[maybe_unused]] uintptr_t *container_offset_iter, [[maybe_unused]] uintptr_t head) override;
    };
}
