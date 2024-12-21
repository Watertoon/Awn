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

namespace awn::para {

    AddressTable *sGlobalAddressTable = nullptr;

    AddressTable *GetAddressTable() {
        return sGlobalAddressTable;
    }
    void SetAddressTable(AddressTable *address_table) {
        sGlobalAddressTable = address_table;
    }
    bool ReplaceAddress(const char **out_address, const char *address) {
        if (sGlobalAddressTable == nullptr) { *out_address = address; return false; }
        return sGlobalAddressTable->ReplaceAddress(out_address, address);
    }

	bool AddressTable::Initialize(mem::Heap *heap, const char *file_path) {

		/* Load heap size database */
        res::FileLoadContext load_context = {
            .heap = heap,
        };
		const Result result = awn::res::LoadFile(file_path, std::addressof(load_context));
		if (RESULT_FAILED(result)) { return false; }
        m_file = load_context.file;

		/* Initialize iterator */
		vp::res::ByamlIterator iter(m_file);
		m_iter = iter;
		if (iter.IsValid() && vp::res::IsDictionaryType(static_cast<vp::res::ByamlDataType>(iter.GetDataType()))) { return true; }

		this->Finalize();

		return false;
	}

	void AddressTable::Finalize() {
		if (m_file != nullptr) {
			::operator delete (m_file);
			m_file = nullptr;
		}
	}

	bool AddressTable::ReplaceAddress(const char **out_address, const char *address) {
        const bool result = m_iter.TryGetStringByKey(out_address, address);
		if (!result) { *out_address = address; }
        return result;
	}
}
