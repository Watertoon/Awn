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
	
    class IDocumentApplicationImpl {
        public:
            constexpr IDocumentApplicationImpl() {/*...*/}
            virtual ~IDocumentApplicationImpl() {/*...*/}
            
            virtual Document *TryLoadDocument(const char *file_name, const char *class_name, bool is_allow_dummy) { VP_UNUSED(file_name, class_name, is_allow_dummy); return nullptr; }
            virtual void DestructDocument(Document *document)  { VP_UNUSED(document); }
            virtual void OnBindDocument(Document *document)    { VP_UNUSED(document); }
            virtual bool OnLoadDocument(Document *document) { VP_UNUSED(document); return true; }
            virtual bool OnReleaseDocument(Document *document) { VP_UNUSED(document); return true; }
            virtual constexpr bool IsResolveLoad(Document *document, const char *path) { VP_UNUSED(document, path); return true; }
    };
}
