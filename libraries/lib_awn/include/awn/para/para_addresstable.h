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

    bool ReplaceAddress(const char **out_address, const char *address);

    class AddressTable {
        private:
            void                   *m_file;
            vp::res::ByamlIterator  m_iter;
        public:
            constexpr  AddressTable() : m_file(), m_iter() {/*...*/}
            constexpr ~AddressTable() {/*...*/}

            bool Initialize(mem::Heap *heap, const char *file_path);
            void Finalize();

            bool ReplaceAddress(const char **out_address, const char *address);
    };

    AddressTable *GetAddressTable();
    void SetAddressTable(AddressTable *address_table);
}
