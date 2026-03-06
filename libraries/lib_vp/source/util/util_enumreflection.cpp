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
#include <vp.hpp>

namespace vp::util {

    constinit EnumDictionaryManager sEnumDictionaryManagerInstance;

    const EnumDictionaryManager *GetEnumDictionaryManager() {
        return std::addressof(sEnumDictionaryManagerInstance);
    }

    EnumDictionaryNode::EnumDictionaryNode(const EnumDictionaryBase *enum_dictionary) : m_tree_node(this, vp::util::HashMurmur64(enum_dictionary->GetClassName())), m_dictionary(enum_dictionary) {
        sEnumDictionaryManagerInstance.Insert(this);
    }
}
