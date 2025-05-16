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

namespace awn::grr {
    class NativeWindow;
}

namespace awn::grr::impl {

    enum class DropDataType {
        Null = 0,
        String = 1,
        String16 = 2,
        FilePathArray = 3,
    };

    class DropTargetHandler;

    class DropData {
        public:
            friend class DropTargetHandler;
            friend class grr::NativeWindow;
        private:
            void *m_data;
            size_t m_data_size;
            DropDataType m_data_type;
        private:
            static bool IsValidFormat(FORMATETC *etc);
            void CopyData(FORMATETC *etc, STGMEDIUM *medium);
            void ParseDataSource(FORMATETC *etc, STGMEDIUM *medium);
        public:
            constexpr DropData() : m_data(), m_data_size(), m_data_type() {/*...*/}
            constexpr ~DropData() {/*...*/}

            void Finalize();
        public:
            constexpr void *GetData() { return m_data; }
            constexpr size_t GetDataSize() { return m_data_size; }
            constexpr DropDataType GetDataType() { return m_data_type; }
    };

    class DropTargetHandler : public IDropTarget {
        public:
            friend class grr::NativeWindow;
        private:
            NativeWindow *m_window;
            bool m_is_drop;
            bool m_is_accept;
            u32 m_data_count;
            DropData *m_data_array;
        public:
            constexpr DropTargetHandler() : m_window(), m_is_drop(), m_is_accept(), m_data_count(), m_data_array() {/*...*/}
            virtual ~DropTargetHandler() {/*...*/}

            constexpr void SetIsAccept(bool is_accept) { m_is_accept = is_accept; }
        public:
            virtual HRESULT DragEnter(IDataObject *obj, DWORD key_state, POINTL point, DWORD *out_effect) override;
            virtual HRESULT DragLeave() override;
            virtual HRESULT DragOver(DWORD key_state, POINTL point, DWORD *out_effect) override;
            virtual HRESULT Drop(IDataObject *obj, DWORD key_state, POINTL point, DWORD *out_effect) override;
    };
}
