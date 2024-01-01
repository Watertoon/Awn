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

    class ResourceUserInfo {
        public:
            VP_RTTI_BASE(ResourceUserInfo);
        public:
            constexpr ResourceUserInfo() {/*...*/}
            virtual ~ResourceUserInfo() {/*...*/}
    };

    class ResourceFactoryBase;

    class Resource {
        public:
            friend class ResourceFactoryBase;
        protected:
            void             *m_file;
            size_t            m_file_size;
            ResourceUserInfo *m_user_info;
        public:
            VP_RTTI_BASE(Resource);
        public:
            constexpr ALWAYS_INLINE Resource() : m_file(), m_file_size(), m_user_info() {/*...*/}
            virtual ~Resource() {/*...*/}

            virtual Result OnFileLoad([[maybe_unused]] mem::Heap *heap, [[maybe_unused]] void *file, [[maybe_unused]] size_t file_size)         { return true; }

            virtual Result InitializeForAsync([[maybe_unused]] mem::Heap *heap, [[maybe_unused]] void *file, [[maybe_unused]] size_t file_size) { return true; }
            virtual void   FinalizeForAsync()                                                                                                   { return; }

            constexpr ALWAYS_INLINE void *GetFile()           { return m_file; }
            constexpr ALWAYS_INLINE u32   GetFileSize() const { return m_file_size; }

            constexpr ALWAYS_INLINE void SetUserInfo(ResourceUserInfo *user_info) { m_user_info = user_info; }
    };
}
