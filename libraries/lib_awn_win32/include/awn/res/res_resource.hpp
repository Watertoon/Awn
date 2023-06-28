#pragma once

namespace awn::res {

    class Resource {
        protected:
            void *m_file;
            u32   m_file_size;
        public:
            VP_RTTI_BASE(Resource);
        public:
            constexpr ALWAYS_INLINE Resource() : m_file(nullptr), m_file_size(0) {/*...*/}
            constexpr virtual ~Resource() {/*...*/}

            virtual bool Initialize(mem::Heap *heap, void *file, u32 file_size) { VP_UNUSED(heap, file, file_size); return false; }
            virtual void Finalize() { return; }

            constexpr virtual size_t GetFileAlignment()        const { return alignof(u32); }

            constexpr ALWAYS_INLINE void *GetFile()           { return m_file; }
            constexpr ALWAYS_INLINE u32   GetFileSize() const { return m_file_size; }
    };

    
}
