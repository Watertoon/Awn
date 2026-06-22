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

namespace awn::gfx {

    void ResourceSet::BindImpl(gfx::CommandBuffer *buffer) const {
        /* Push data */
        const VkPushDataInfoEXT info = {
            .sType = VK_STRUCTURE_TYPE_PUSH_DATA_INFO_EXT,
            .offset = 0,
            .data = {
                .address = std::addressof(m_gpu_address.vk_device_address),
                .size = sizeof(VkDeviceAddress),
            },
        };
        ::pfn_vkCmdPushDataEXT(buffer->GetVkCommandBuffer(), std::addressof(info));
        return;
    }
}
