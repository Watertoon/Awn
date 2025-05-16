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
#include <awn.hpp>

namespace awn::grr::impl {

    void DropData::CopyData(FORMATETC *etc, STGMEDIUM *medium) {

        const CLIPFORMAT format = etc->cfFormat;
        const u32 tyme = etc->tymed;
        if (tyme == TYMED_HGLOBAL) {

            if (format == CF_HDROP) {                        
                /* Copy hdrop */
                HDROP hdrop = reinterpret_cast<HDROP>(medium->hGlobal);
                const u32 file_count = ::DragQueryFileA(hdrop, 0xffff'ffff, nullptr, 0);
                res::MaxPathString *max_path_array = reinterpret_cast<res::MaxPathString*>(::malloc(sizeof(res::MaxPathString) * file_count));
                for (u32 i = 0; i < file_count; ++i) {
                    char *path = max_path_array[i].GetString();
                    /* Zero is returned if invalid, stubbing the string */
                    const u32 wrote = ::DragQueryFileA(hdrop, i, path, 0xff);
                    max_path_array[i].SetLength(wrote);
                    max_path_array[i].AssureTermination();
                }
                m_data = max_path_array;
                m_data_size = file_count;
                m_data_type = DropDataType::FilePathArray;
            } else {
                
                /* Copy global memory */
                HGLOBAL hglobal = medium->hGlobal;
                const size_t size = ::GlobalSize(hglobal);
                if (size == 0) { return; }
                void *memory = ::malloc(size);
                if (memory == nullptr) { return; }
                void *gmem = ::GlobalLock(hglobal);
                ::memcpy(memory, gmem, size);
                ::GlobalUnlock(hglobal);
                if (format == CF_TEXT) {                            
                    m_data_type = DropDataType::String;
                } else if (format == CF_UNICODETEXT) {
                    m_data_type = DropDataType::String16;
                }
            }
        }

        return;
    }

    bool DropData::IsValidFormat(FORMATETC *etc) {

        /* Check for supportted data type */
        const CLIPFORMAT format = etc->cfFormat;
        const u32 tyme = etc->tymed;
        if (format != CF_HDROP && format != CF_TEXT && format != CF_UNICODETEXT) { return false; }
        if (tyme != TYMED_HGLOBAL) { return false; }

        return true;
    }

    void DropData::ParseDataSource(FORMATETC *etc, STGMEDIUM *medium) {

        /* Copy data type */
        this->CopyData(etc, medium);

        return;
    }

    void DropData::Finalize() {
        if (m_data != nullptr) {
            ::free(m_data);
            m_data = nullptr;
            m_data_size = 0;
            m_data_type = DropDataType::Null;
        }
        return;
    }

    HRESULT DropTargetHandler::DragEnter(IDataObject *obj, DWORD key_state, POINTL point, DWORD *out_effect) {
        VP_UNUSED(point);

        /* Ensure out ptr */
        if (out_effect == nullptr) { return E_INVALIDARG; }
        auto fail_guard0 = SCOPE_GUARD {
            *out_effect = DROPEFFECT_NONE;
        };

        /* Drop guard */
        {
            std::scoped_lock l(m_window->m_event_cs);
            if (!m_is_accept) { return S_OK; }
            if (m_is_drop) { return S_OK; }
        }

        /* Check to see if we accept anything we are being passed */
        IEnumFORMATETC *enum_format;
        const HRESULT hresult0 = obj->EnumFormatEtc(DATADIR_GET, std::addressof(enum_format));
        if (hresult0 != S_OK) { return E_UNEXPECTED; }
        ON_SCOPE_EXIT {
            enum_format->Release();
        };
        FORMATETC format_array[8];
        bool is_accept_any = false;
        for (;;) {
            ULONG out_count = 0;
            enum_format->Next(8, format_array, std::addressof(out_count));
            if (out_count == 0) { break; }
            for (u32 i = 0; i < out_count; ++i) {

                /* Query whether the data is valid */
                FORMATETC *etc = std::addressof(format_array[i]);
                if (obj->QueryGetData(etc) != S_OK) { continue; }
                if (!DropData::IsValidFormat(etc)) { continue; }
                is_accept_any = true;
                break;
            }
            if (is_accept_any) { break; }
        }
        if (!is_accept_any) { return S_OK; }

        /* Tell source process our drop effect */
        fail_guard0.Cancel();
        if ((key_state & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT)) {
            *out_effect = DROPEFFECT_LINK;
        } else if ((key_state & MK_CONTROL) == MK_CONTROL) {
            *out_effect = DROPEFFECT_COPY;
        } else {
            *out_effect = DROPEFFECT_MOVE;
        }

        return S_OK;
    }

    HRESULT DropTargetHandler::DragLeave() {
        return S_OK;
    }

    HRESULT DropTargetHandler::DragOver(DWORD key_state, POINTL point, DWORD *out_effect) {
        VP_UNUSED(point);

        /* Ensure out ptr */
        if (out_effect == nullptr) { return E_INVALIDARG; }
        auto fail_guard0 = SCOPE_GUARD {
            *out_effect = DROPEFFECT_NONE;
        };

        /* Drop guard */
        {
            std::scoped_lock l(m_window->m_event_cs);
            if (!m_is_accept) { return S_OK; }
            if (m_is_drop) { return S_OK; }
        }

        /* Retell source process our drop effect */
        fail_guard0.Cancel();
        if ((key_state & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT)) {
            *out_effect = DROPEFFECT_LINK;
        } else if ((key_state & MK_CONTROL) == MK_CONTROL) {
            *out_effect = DROPEFFECT_COPY;
        } else {
            *out_effect = DROPEFFECT_MOVE;
        }

        return S_OK;
    }

    HRESULT DropTargetHandler::Drop(IDataObject *obj, DWORD key_state, POINTL point, DWORD *out_effect) {
        VP_UNUSED(point);

        /* Ensure out ptr */
        if (out_effect == nullptr) { return E_INVALIDARG; }

        /* Drop guard */
        {
            std::scoped_lock l(m_window->m_event_cs);
            if (!m_is_accept || m_is_drop) { *out_effect = DROPEFFECT_NONE; return S_OK; }
            m_is_drop = true;
        }

        /* Count valid clipboard data */
        IEnumFORMATETC *enum_format;
        const HRESULT hresult0 = obj->EnumFormatEtc(DATADIR_GET, std::addressof(enum_format));
        if (hresult0 != S_OK) { return E_UNEXPECTED; }
        FORMATETC format_array[8];
        u32 count = 0;
        //bool is_accept_any = false;
        for (;;) {
            ULONG out_count = 0;
            enum_format->Next(8, format_array, std::addressof(out_count));
            if (out_count == 0) { break; }
            for (u32 i = 0; i < out_count; ++i) {

                /* Query whether the data is valid */
                FORMATETC *etc = std::addressof(format_array[i]);
                if (obj->QueryGetData(etc) != S_OK) { continue; }
                if (!DropData::IsValidFormat(etc)) { continue; }
                ++count;
            }
        }
        
        /* Create data array */
        m_data_array = reinterpret_cast<DropData*>(::malloc(sizeof(DropData) * count));
        for (u32 i = 0; i < count; ++i) {
            std::construct_at(std::addressof(m_data_array[i]));
        }

        /* Parse data */
        enum_format->Reset();
        u32 i = 0;
        for (;;) {
            ULONG out_count = 0;
            enum_format->Next(8, format_array, std::addressof(out_count));
            if (out_count == 0) { break; }
            for (u32 i = 0; i < out_count; ++i) {
                if (count <= i) { break; }

                /* Query whether the data is valid */
                FORMATETC *etc = std::addressof(format_array[i]);
                if (obj->QueryGetData(etc) != S_OK) { continue; }
                if (!DropData::IsValidFormat(etc)) { continue; }
                STGMEDIUM medium;
                const HRESULT result = obj->GetData(etc, std::addressof(medium));
                if (result != S_OK) { continue; }
                m_data_array[i].ParseDataSource(etc, std::addressof(medium));
                ::ReleaseStgMedium(std::addressof(medium));
                ++i;
            }
        }
        m_data_count = i;

        if ((key_state & (MK_CONTROL | MK_SHIFT)) == (MK_CONTROL | MK_SHIFT)) {
            *out_effect = DROPEFFECT_LINK;
        } else if ((key_state & MK_CONTROL) == MK_CONTROL) {
            *out_effect = DROPEFFECT_COPY;
        } else {
            *out_effect = DROPEFFECT_MOVE;
        }

        return S_OK;
    }
}
