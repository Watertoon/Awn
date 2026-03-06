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

namespace awn::para {

    namespace impl {
        class DocumentQueue;
    }

    enum class DocumentPathType {
        Invalid = 0,
        Path = 1,
        NameAndClass = 2,
    };

    struct DocumentBinderAsyncInfo {
        DocumentPathType path_type;
        bool is_allow_dummy;
        const char *path;
        const char *class_name;
        
        constexpr void SetDefaults() {
            this->path_type = DocumentPathType::Invalid;
            this->is_allow_dummy = true;
            this->path = "";
            this->class_name = "";
        }
        constexpr void SetFilePath(const char *path) {
            this->path_type = DocumentPathType::Path;
            this->path = path;
        }
        constexpr void SetNameAndClass(const char *name, const char *class_name) {
            this->path_type = DocumentPathType::NameAndClass;
            this->path = name;
            this->class_name = class_name;
        }
        constexpr void SetIsAllowDummy(bool is_allow_dummy) {
            this->is_allow_dummy = is_allow_dummy;
        }
    };

    class DocumentBinderAsync {
        public:
            friend class impl::DocumentQueue;
        private:
            async::AsyncTaskWatcher m_watcher;
            DocumentBinder m_binder;
        public:
            constexpr DocumentBinderAsync() : m_watcher(), m_binder() {/*...*/}
            ~DocumentBinderAsync() {/*...*/}

            Result LoadDocumentAsync(const DocumentBinderAsyncInfo *info);
            void WaitForCompletion();

            constexpr DocumentBinder *GetDocumentBinder() {
                return std::addressof(m_binder);
            }
    };
}
