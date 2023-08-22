#pragma once

namespace vp::util {

    template <class ...Args>
    class IDelegate {
        public:
            constexpr ALWAYS_INLINE  IDelegate() {/*...*/}
            constexpr ALWAYS_INLINE ~IDelegate() {/*...*/}

            virtual constexpr inline void Invoke(Args... args) = 0;
            virtual constexpr inline void TryInvoke(Args... args) = 0;
    };

    template <class ParentType, class ...Args>
    class Delegate : public IDelegate<Args...> {
        public:
            using FunctionType = void (ParentType::*)(Args...);
        public:
            FunctionType  m_function;
            ParentType   *m_parent;
        public:
            constexpr ALWAYS_INLINE Delegate() : m_function(nullptr), m_parent(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE Delegate(FunctionType function) : m_function(function), m_parent(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE Delegate(ParentType *parent) : m_function(nullptr), m_parent(parent) {/*...*/}
            constexpr ALWAYS_INLINE Delegate(ParentType *parent, FunctionType function) : m_function(function), m_parent(parent) {/*...*/}
            constexpr ~Delegate() {/*...*/}

            virtual constexpr inline void Invoke(Args... args) override {
                VP_ASSERT(m_parent != nullptr && m_function != nullptr);
                (m_parent->*m_function)(std::forward<Args>(args) ...);
            }

            virtual constexpr inline void TryInvoke(Args... args) override {
                if (m_parent == nullptr || m_function == nullptr) { return; }
                (m_parent->*m_function)(std::forward<Args>(args) ...);
            }
    };

    template <class ...Args>
    class DelegateFunction : public IDelegate<Args...> {
        public:
            using FunctionType = void (*)(Args...);
        public:
            FunctionType  m_function;
        public:
            constexpr ALWAYS_INLINE DelegateFunction() : m_function(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE DelegateFunction(FunctionType function) : m_function(function) {/*...*/}
            constexpr ~DelegateFunction() {/*...*/}

            virtual constexpr inline void Invoke(Args... args) override {
                VP_ASSERT(m_function != nullptr);
                (*m_function)(std::forward<Args>(args) ...);
            }

            virtual constexpr inline void TryInvoke(Args... args) override {
                if (m_function != nullptr) { return; }
                (*m_function)(std::forward<Args>(args) ...);
            }
    };

    template <class ReturnType, class ...Args>
    class IDelegateReturn {
        public:
            constexpr ALWAYS_INLINE  IDelegateReturn() {/*...*/}
            constexpr ALWAYS_INLINE ~IDelegateReturn() {/*...*/}

            virtual constexpr inline ReturnType Invoke(Args... args) = 0;
            virtual constexpr inline ReturnType TryInvoke(Args... args) = 0;
    };

    template <class ReturnType, class ParentType, class ...Args>
    class DelegateReturn : public IDelegateReturn<ReturnType, Args...> {
        public:
            using FunctionType = ReturnType (ParentType::*)(Args...);
        public:
            FunctionType  m_function;
            ParentType   *m_parent;
        public:
            constexpr ALWAYS_INLINE DelegateReturn() : m_function(nullptr), m_parent(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE DelegateReturn(FunctionType function) : m_function(function), m_parent(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE DelegateReturn(ParentType *parent) : m_function(nullptr), m_parent(parent) {/*...*/}
            constexpr ALWAYS_INLINE DelegateReturn(ParentType *parent, FunctionType function) : m_function(function), m_parent(parent) {/*...*/}
            constexpr ~DelegateReturn() {/*...*/}

            virtual constexpr inline ReturnType Invoke(Args... args) override {
                VP_ASSERT(m_parent != nullptr && m_function != nullptr);
                return (m_parent->*m_function)(std::forward<Args>(args) ...);
            }

            virtual constexpr inline ReturnType TryInvoke(Args... args) override {
                if (m_parent == nullptr || m_function == nullptr) { return; }
                return (m_parent->*m_function)(std::forward<Args>(args) ...);
            }
    };

    template <class ReturnType, class ...Args>
    class DelegateReturnFunction : public IDelegateReturn<ReturnType, Args...> {
        public:
            using FunctionType = ReturnType (*)(Args...);
        public:
            FunctionType  m_function;
        public:
            constexpr ALWAYS_INLINE DelegateReturnFunction() : m_function(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE DelegateReturnFunction(FunctionType function) : m_function(function) {/*...*/}
            constexpr ~DelegateReturnFunction() {/*...*/}

            virtual constexpr inline ReturnType Invoke(Args... args) override {
                VP_ASSERT(m_function != nullptr);
                return (*m_function)(std::forward<Args>(args) ...);
            }

            virtual constexpr inline ReturnType TryInvoke(Args... args) override {
                if (m_function != nullptr) { return 0; }
                return (*m_function)(std::forward<Args>(args) ...);
            }
    };
}
