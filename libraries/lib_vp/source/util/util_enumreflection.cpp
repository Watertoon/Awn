#include <vp.hpp>

namespace vp::res {

    EnumDictionaryNode::EnumDictionaryNode(const EnumDictionary *enum_dictionary) : m_tree_node(vp::util::HashMurmur64(enum_dictionary->GetClassName())), m_dictionary(enum_dictionary)  {
        vp::res::GetEnumDictionaryManager()->Insert(this);
    }

    constinit EnumDictionaryManager sEnumDictionaryManagerInstance;

    EnumDictionaryManager *GetEnumDictionaryManager() { return std::addressof(sEnumDictionaryManagerInstance); }
}
