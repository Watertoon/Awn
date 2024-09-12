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

    void SubmitCommandList(CommandList command_list, Sync **wait_sync_array, u32 wait_sync_count, Sync **signal_sync_array, u32 signal_sync_count) {
        impl::Context::GetInstance()->SubmitCommandList(command_list, wait_sync_array, wait_sync_count, signal_sync_array, signal_sync_count);
    }

    void SubmitCommandLists(CommandList **command_list_array, u32 command_list_count, Sync **wait_sync_array, u32 wait_sync_count, Sync **signal_sync_array, u32 signal_sync_count) {
        impl::Context::GetInstance()->SubmitCommandLists(command_list_array, command_list_count, wait_sync_array, wait_sync_count, signal_sync_array, signal_sync_count);
    }

    void FinalizeCommandList(CommandList command_list) {
        impl::CommandPoolManager::GetInstance()->FinalizeCommandList(command_list);
    }
}
