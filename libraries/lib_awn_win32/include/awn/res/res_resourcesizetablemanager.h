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

	class ResourceSizeTableManager {
        public:
            static constexpr u32 cInvalidSize = vp::res::ResourceSizeTableExtractor::cInvalidSize;
        public:
            struct RsizetableNode {
                vp::util::IntrusiveListNode         list_node;
                vp::res::ResourceSizeTableExtractor extractor;
            };
        public:
            using ResourceSizeTableList = vp::util::IntrusiveListTraits<RsizetableNode, &RsizetableNode::list_node>::List;
		private:
			vp::res::ResourceSizeTableExtractor  m_extractor;
			ResourceSizeTableList                m_fallback_list;
            Resource                            *m_resource;
		public:
			constexpr ResourceSizeTableManager() : m_extractor(), m_fallback_list(), m_resource() {/*...*/}
			constexpr ~ResourceSizeTableManager() {/*...*/}

			void Initialize(const char *path);
			void Finalize();
			void Clear();

            u32 GetResourceSize(const char *path);

			bool RegisterResourceSizeTable(mem::Heap *heap, void *rsizetable, u32 rsizetable_size);
			void UnregisterResourceSizeTable(void *rsizetable);
	};
}
