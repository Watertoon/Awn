#pragma once
    
namespace vp::util {

    class Job {
        private:
            const char *m_job_name;
        public:
            constexpr ALWAYS_INLINE Job() : m_job_name(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE Job(const char *name) : m_job_name(name) {/*...*/}

            virtual void Invoke() {/*...*/}

            constexpr ALWAYS_INLINE void SetName(const char *name) { m_job_name = name; }
    };
}
