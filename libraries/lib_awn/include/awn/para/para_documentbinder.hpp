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

    class Document;
    class TypedParam;

    class DocumentBinder {
        public:
            friend class TypedParam;
        private:
            Document *m_document;
        protected:
            void LoadDocumentForResolve(const char *format_path);
        public:
            constexpr DocumentBinder() : m_document() {/*...*/}
            ~DocumentBinder() { this->Release(); }

            constexpr bool IsValid() const { return m_document != nullptr; }

            DocumentBinder &operator=(const DocumentBinder &rhs);

            void LoadDocument(const char *file_name, const char *class_name, bool is_allow_dummy);
            void Release();

            /* Defined in para_document.hpp */
            template <typename T>
            ALWAYS_INLINE T *GetTypedParam() const;
    };
}
