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

namespace awn::res {

    class IComposeInfo {
        public:
            VP_RTTI_BASE(IComposeInfo);
        public:
            constexpr IComposeInfo() {/*...*/}
            virtual ~IComposeInfo() {/*...*/}
    };

    class ResourceFactoryBase;

    class Resource {
        public:
            friend class ResourceFactoryBase;
        protected:
            void *m_file;
            size_t m_file_size;
        public:
            VP_RTTI_BASE(Resource);
        public:
            constexpr ALWAYS_INLINE Resource() : m_file(), m_file_size() {/*...*/}
            virtual constexpr ~Resource() {/*...*/}

            virtual Result OnFileLoad(mem::Heap *heap, void *file, size_t file_size) { VP_UNUSED(heap, file, file_size); RESULT_RETURN_SUCCESS; }
            constexpr ALWAYS_INLINE void *GetFile()           { return m_file; }
            constexpr ALWAYS_INLINE u32   GetFileSize() const { return m_file_size; }
        public:
            virtual constexpr bool IsRequireCompose() const { return false; }

            virtual Result Compose(mem::Heap *heap, IComposeInfo *info, void *file, size_t file_size) { VP_UNUSED(heap, info, file, file_size); RESULT_RETURN_SUCCESS; }
            virtual Result PostCompose(mem::Heap *heap, IComposeInfo *info, void *file, size_t file_size) { VP_UNUSED(heap, info, file, file_size); RESULT_RETURN_SUCCESS; }
            virtual Result PreFinalize() { RESULT_RETURN_SUCCESS; }
            virtual void Finalize() { return; }
    };
}
