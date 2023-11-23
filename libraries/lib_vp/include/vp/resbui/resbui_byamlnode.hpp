#pragma once

namespace vp::resbui {

    class ByamlBigDataNodeBase;
    class ByamlBigDataNodeContainerBase;
    class ByamlBigDataNodeArray;
    class ByamlBigDataNodeDictionary;
    class ByamlBigDataNodeHashArray;

	class ByamlNodeBase {
        public:
            friend class ByamlBigDataNodeBase;
            friend class ByamlBigDataNodeContainerBase;
            friend class ByamlBigDataNodeArray;
            friend class ByamlBigDataNodeDictionary;
            friend class ByamlBigDataNodeHashArray;
		public:
			static constexpr u64 cInvalidHash  = 0xffff'ffff'ffff'ffff;
			static constexpr u32 cInvalidIndex = 0xffff'ffff;
		private:
            vp::util::IntrusiveListNode  m_container_node;
            ByamlStringPoolString        m_key;
            u64                          m_hash;
            u32                          m_index;
        public:
            VP_RTTI_BASE(ByamlNodeBase);
		public:
			constexpr ByamlNodeBase() : m_container_node(), m_key(""),  m_hash(cInvalidHash), m_index(cInvalidIndex) {/*...*/}
            virtual constexpr  ~ByamlNodeBase() {/*...*/}

            constexpr const ByamlStringPoolString *GetKey()   const { return std::addressof(m_key); }
            constexpr u64                          GetHash()  const { return m_hash; }
            constexpr u32                          GetIndex() const { return m_index; }

            virtual void PushKeys(ByamlStringPoolBuilder *pool_builder) {
                if (*m_key.GetString() == '/0') { return; }
                pool_builder->AddString(std::addressof(m_key));
            }
            virtual void PushStrings([[maybe_unused]] ByamlStringPoolBuilder *pool_builder) {/*...*/}

            virtual constexpr res::ByamlDataType GetByamlDataType() const { return res::ByamlDataType::Null; }
            virtual void Serialize(void *data_offset, vp::res::ResByaml *head) {
                /* Write null */
                *reinterpret_cast<u32*>(data_offset) = 0;
            }
    };

    class ByamlNodeBigData : public ByamlNodeBase {
        private:
            ByamlBigDataNodeBase *m_big_data_node;
        public:
            VP_RTTI_DERIVED(ByamlNodeBigData, ByamlNodeBase);
        public:
			constexpr ByamlNodeBigData() : ByamlNodeBase(), m_big_data_node() {/*...*/}
			constexpr ByamlNodeBigData(ByamlBigDataNodeBase *big_data_node) : ByamlNodeBase(), m_big_data_node(big_data_node) {/*...*/}
            virtual constexpr ~ByamlNodeBigData() {/*...*/}

            virtual void PushKeys(ByamlStringPoolBuilder *pool_builder) override;
            virtual void PushStrings(ByamlStringPoolBuilder *pool_builder) override;

            virtual constexpr res::ByamlDataType GetByamlDataType() const override;
            virtual void Serialize(void *data_offset, vp::res::ResByaml *head) override;

            void SerializeForBigData(uintptr_t *big_data_offset, uintptr_t *container_offset, vp::res::ResByaml *head);

            void CalculateBigDataEndOffsets(uintptr_t *big_data_offset_iter, uintptr_t *container_offset_iter, size_t head);

            constexpr void SetBigDataNode(ByamlBigDataNodeBase *big_data_node) {
                m_big_data_node = big_data_node;
            }
    };

    class ByamlNodeU32 : public ByamlNodeBase {
        private:
            u32 m_value;
        public:
			constexpr ByamlNodeU32() : ByamlNodeBase(), m_value() {/*...*/}
			constexpr ByamlNodeU32(u32 value) : ByamlNodeBase(), m_value(value) {/*...*/}
            virtual constexpr ~ByamlNodeU32() {/*...*/}

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return res::ByamlDataType::U32; }
            virtual void Serialize(void *data_offset, vp::res::ResByaml *head) override {
                *reinterpret_cast<u32*>(data_offset) = m_value;
            }

            constexpr void SetValue(u32 value) {
                m_value = value;
            }
    };
    class ByamlNodeS32 : public ByamlNodeBase {
        private:
            s32 m_value;
        public:
			constexpr ByamlNodeS32() : ByamlNodeBase(), m_value() {/*...*/}
			constexpr ByamlNodeS32(s32 value) : ByamlNodeBase(), m_value(value) {/*...*/}
            virtual constexpr ~ByamlNodeS32() {/*...*/}

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return res::ByamlDataType::S32; }
            virtual void Serialize(void *data_offset, vp::res::ResByaml *head) override {
                *reinterpret_cast<s32*>(data_offset) = m_value;
            }

            constexpr void SetValue(s32 value) {
                m_value = value;
            }
    };
    class ByamlNodeFloat : public ByamlNodeBase {
        private:
            float m_value;
        public:
			constexpr ByamlNodeFloat() : ByamlNodeBase(), m_value() {/*...*/}
			constexpr ByamlNodeFloat(float value) : ByamlNodeBase(), m_value(value) {/*...*/}
            virtual constexpr ~ByamlNodeFloat() {/*...*/}

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return res::ByamlDataType::F32; }
            virtual void Serialize(void *data_offset, vp::res::ResByaml *head) override {
                *reinterpret_cast<float*>(data_offset) = m_value;
            }

            constexpr void SetValue(float value) {
                m_value = value;
            }
    };
    class ByamlNodeBool : public ByamlNodeBase {
        private:
            bool m_value;
        public:
			constexpr ByamlNodeBool() : ByamlNodeBase(), m_value() {/*...*/}
			constexpr ByamlNodeBool(bool value) : ByamlNodeBase(), m_value(value) {/*...*/}
            virtual constexpr ~ByamlNodeBool() {/*...*/}

            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return res::ByamlDataType::Bool; }
            virtual void Serialize(void *data_offset, vp::res::ResByaml *head) override {
                *reinterpret_cast<bool*>(data_offset) = m_value;
            }

            constexpr void SetValue(bool value) {
                m_value = value;
            }
    };
    class ByamlNodeString : public ByamlNodeBase {
        private:
            ByamlStringPoolString m_string;
        public:
			constexpr ByamlNodeString() : ByamlNodeBase(), m_string("") {/*...*/}
			constexpr ByamlNodeString(const char *value) : ByamlNodeBase(), m_string(value) {/*...*/}
            virtual constexpr ~ByamlNodeString() {/*...*/}


            virtual constexpr res::ByamlDataType GetByamlDataType() const override { return res::ByamlDataType::StringIndex; }
            virtual void Serialize(void *data_offset, vp::res::ResByaml *head) override {
                *reinterpret_cast<u32*>(data_offset) = m_string.GetKeyIndex();
            }

            constexpr void SetValue(const char *value) {
                m_string.SetString(value);
            }
    };
}
