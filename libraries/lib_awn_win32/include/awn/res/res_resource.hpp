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
