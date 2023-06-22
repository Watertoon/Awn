#pragma once

namespace vp::util {

    template <class ParentType, class ...Args>
    class Delegate {
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

            constexpr ALWAYS_INLINE void Invoke(Args... args) {
                VP_ASSERT(m_parent != nullptr && m_function != nullptr);
                (m_parent->*m_function)(std::forward<Args>(args) ...);
            }

            constexpr ALWAYS_INLINE void TryInvoke(Args... args) {
                if (m_parent == nullptr || m_function == nullptr) { return; }
                (m_parent->*m_function)(std::forward<Args>(args) ...);
            }
    };

    template <class ReturnType, class ParentType, class ...Args>
    class DelegateReturn {
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

            constexpr ALWAYS_INLINE ReturnType Invoke(Args... args) {
                VP_ASSERT(m_parent != nullptr && m_function != nullptr);
                return (m_parent->*m_function)(std::forward<Args>(args) ...);
            }

            constexpr ALWAYS_INLINE ReturnType TryInvoke(Args... args) {
                if (m_parent == nullptr || m_function == nullptr) { return; }
                return (m_parent->*m_function)(std::forward<Args>(args) ...);
            }
    };

    template <class ...Args>
    class DelegateFunction {
        public:
            using FunctionType = void (*)(Args...);
        public:
            FunctionType  m_function;
        public:
            constexpr ALWAYS_INLINE DelegateFunction() : m_function(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE DelegateFunction(FunctionType function) : m_function(function) {/*...*/}
            constexpr ~DelegateFunction() {/*...*/}

            constexpr ALWAYS_INLINE void Invoke(Args... args) {
                VP_ASSERT(m_function != nullptr);
                (*m_function)(std::forward<Args>(args) ...);
            }

            constexpr ALWAYS_INLINE void TryInvoke(Args... args) {
                if (m_function != nullptr) { return; }
                (*m_function)(std::forward<Args>(args) ...);
            }
    };

    template <class ReturnType, class ...Args>
    class DelegateReturnFunction {
        public:
            using FunctionType = ReturnType (*)(Args...);
        public:
            FunctionType  m_function;
        public:
            constexpr ALWAYS_INLINE DelegateReturnFunction() : m_function(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE DelegateReturnFunction(FunctionType function) : m_function(function) {/*...*/}
            constexpr ~DelegateReturnFunction() {/*...*/}

            constexpr ALWAYS_INLINE ReturnType Invoke(Args... args) {
                VP_ASSERT(m_function != nullptr);
                return (*m_function)(std::forward<Args>(args) ...);
            }

            constexpr ALWAYS_INLINE ReturnType TryInvoke(Args... args) {
                if (m_function != nullptr) { return; }
                return (*m_function)(std::forward<Args>(args) ...);
            }
    };
}
