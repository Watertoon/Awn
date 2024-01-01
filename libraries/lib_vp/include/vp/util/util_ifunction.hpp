#pragma once

namespace vp::util {

    template <size_t StorageU64Count>
    class Any {
        public:
            struct Storage {
                u64 m_storage[StorageU64Count];
            };
        public:
            constexpr void *operator new(size_t size, Storage *storage) {
                return storage;
            }
        private:
            Storage m_storage;
        public:
            constexpr ALWAYS_INLINE Any() {/*...*/}
            constexpr ALWAYS_INLINE ~Any() {/*...*/}

            template <typename T>
                requires (sizeof(T) <= sizeof(Storage))
            constexpr ALWAYS_INLINE Any(const T &rhs) { ::new (std::addressof(m_storage)) T(rhs);}

            template <typename T>
                requires (sizeof(T) <= sizeof(Storage))
            ALWAYS_INLINE T *GetType() { return reinterpret_cast<T*>(std::addressof(m_storage)); }
            template <typename T>
                requires (sizeof(T) <= sizeof(Storage))
            ALWAYS_INLINE T *GetType() const { return reinterpret_cast<T*>(std::addressof(m_storage)); }
    };

    namespace impl {        
        class AnyFunctionBase {};
    }

    template <class>
    class IFunction;

    template <class Return, class ...Args>
    class IFunction<Return(Args...)> {
        public:
            template <size_t, class>
            friend class AnyFunction;
        public:
            static constexpr bool cIsIFunction = true;
        private:
            constexpr ALWAYS_INLINE void *operator new(size_t size, impl::AnyFunctionBase *any_function) {
                return any_function;
            }
        public:
            constexpr ALWAYS_INLINE IFunction() {/*...*/}
            virtual ~IFunction() {/*...*/}

            virtual Return Invoke(Args ...args) { return Return(); }

            constexpr virtual bool IsValid() const { return false; }
        protected:
            constexpr virtual void CopyTo(impl::AnyFunctionBase *any_function) const {
                new (any_function) IFunction(*this);
            }
    };

    template <class>
    class StaticFunction;

    template <class Return, class ...Args>
    class StaticFunction<Return(Args...)> : public IFunction<Return(Args...)> {
        public:
            template <size_t, class>
            friend class AnyFunction;
        public:
            using FunctionType = Return(*)(Args...);
        private:
            FunctionType m_function;
        private:
            constexpr ALWAYS_INLINE void *operator new(size_t size, impl::AnyFunctionBase *any_function) {
                return any_function;
            }
        public:
            constexpr ALWAYS_INLINE StaticFunction(FunctionType function) : m_function(function) {/*...*/}
            virtual ~StaticFunction() override {/*...*/}

            virtual Return Invoke(Args ...args) override {
                return (m_function)(args...);
            }

            constexpr virtual bool IsValid() const override { return true; }
        protected:
            constexpr virtual void CopyTo(impl::AnyFunctionBase *any_function) const override {
                new (any_function) StaticFunction(*this);
            }
    };

    template <class, class>
    class MemberFunction;

    template <class Parent, class Return, class ...Args>
    class MemberFunction<Parent, Return(Args...)> : public IFunction<Return(Args...)> {
        public:
            template <size_t, class>
            friend class AnyFunction;
        public:
            using FunctionType = Return(Parent::*)(Args...);
            using ParentType   = Parent;
        public:
            struct ParentData {
                ParentType   *parent;
                FunctionType  function;
            };
        private:
            ParentType   *m_parent;
            FunctionType  m_function;
        private:
            constexpr ALWAYS_INLINE void *operator new(size_t size, impl::AnyFunctionBase *any_function) {
                return any_function;
            }
        public:
            constexpr ALWAYS_INLINE MemberFunction(ParentType *parent, FunctionType function) : m_parent(parent), m_function(function) {/*...*/}
            virtual ~MemberFunction() override {/*...*/}

            virtual Return Invoke(Args ...args) override {
                (m_parent->*m_function)(args...);
            }

            constexpr virtual bool IsValid() const override { return true; }
        protected:
            constexpr virtual void CopyTo(impl::AnyFunctionBase *any_function) const override {
                new (any_function) MemberFunction(*this);
            }
    };

    template <class, class>
    class LambdaFunction;

    template <class Lambda, class Return, class ...Args>
    class LambdaFunction<Lambda, Return(Args...)> : public IFunction<Return(Args...)> {
        public:
            template <size_t, class>
            friend class AnyFunction;
        public:
            using FunctionType = Return(*)(Args...);
        private:
            constexpr ALWAYS_INLINE void *operator new(size_t size, impl::AnyFunctionBase *any_function) {
                return any_function;
            }
        private:
            Lambda m_lambda;
        public:
            constexpr ALWAYS_INLINE LambdaFunction(const Lambda&& lambda) : m_lambda(lambda) {/*...*/}
            virtual ~LambdaFunction() override {/*...*/}

            virtual Return Invoke(Args ...args) override {
                return (m_lambda)(args...);
            }

            constexpr virtual bool IsValid() const override { return true; }
        protected:
            constexpr virtual void CopyTo(impl::AnyFunctionBase *any_function) const override {
                new (any_function) LambdaFunction(*this);
            }
    };

    template <size_t, class>
    class AnyFunction;

    template <size_t StorageU64Count, class Return, class ...Args>
    class AnyFunction<StorageU64Count, Return(Args...)> : public impl::AnyFunctionBase {
        private:
            class UnbindDummy : public IFunction<Return(Args...)> {public:
                public:
                    template <size_t, class>
                    friend class AnyFunction;
                private:
                    constexpr ALWAYS_INLINE void *operator new(size_t size, impl::AnyFunctionBase *any_function) {
                        return any_function;
                    }
                public:
                    constexpr ALWAYS_INLINE UnbindDummy() : IFunction<Return(Args...)>() {/*...*/}
                    virtual ~UnbindDummy() override {/*...*/}

                    virtual Return Invoke(Args ...args) override { return Return(); }
                    constexpr virtual bool IsValid() const override { return false; }
                protected:
                    constexpr virtual void CopyTo(impl::AnyFunctionBase *any_function) const override {
                        new (any_function) UnbindDummy(*this);
                    }
            };
        private:
            Any<StorageU64Count> m_any;
            static_assert(sizeof(UnbindDummy) <= sizeof(m_any));
        public:
            constexpr ALWAYS_INLINE AnyFunction() : m_any(std::move(UnbindDummy{})) {/*...*/}
            ALWAYS_INLINE ~AnyFunction() { std::destroy_at(m_any.template GetType<IFunction<Return(Args...)>>()); }

            ALWAYS_INLINE Return Invoke(Args ...args) {
                return m_any.template GetType<IFunction<Return(Args...)>>()->Invoke(args...);
            }
            ALWAYS_INLINE bool IsValid() const {
                return m_any.template GetType<IFunction<Return(Args...)>>()->IsValid();
            }

            template <typename T>
                requires (T::cIsIFunction) && (sizeof(T) <= sizeof(m_any))
            constexpr ALWAYS_INLINE void SetFunction(const T &function) {
                function.CopyTo(this);
            }
    };

    template <class FunctionType, class Lambda>
        requires (sizeof(Lambda) <= 0x18)
    constexpr ALWAYS_INLINE auto MakeLambdaFunction(const Lambda &lambda) {
        return LambdaFunction<Lambda, FunctionType>(std::move(lambda));
    }

    template <class FunctionType, class Parent, class Member>
    constexpr ALWAYS_INLINE auto MakeMemberFunction(Parent *parent, const Member &member_function) {
        return MemberFunction<Parent, FunctionType>(parent, member_function);
    }
    template <class FunctionType, class Function>
    constexpr ALWAYS_INLINE auto MakeStaticFunction(const Function &function) {
        return StaticFunction<FunctionType>(std::move(function));
    }
}
