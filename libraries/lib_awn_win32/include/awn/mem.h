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

#include <awn/sys/sys_servicecriticalsection.win32.hpp>

#include <awn/mem/mem_heapmanager.h>
#include <awn/mem/mem_idisposer.h>
#include <awn/mem/mem_heap.hpp>
#include <awn/mem/mem_idisposer.hpp>
#include <awn/mem/mem_expheap.hpp>
#include <awn/mem/mem_separateheap.hpp>
#include <awn/mem/mem_virtualaddressheap.win32.hpp>

#include <awn/mem/mem_singleton.h>
#include <awn/mem/impl/mem_new.hpp>

#include <awn/mem/mem_gpuexpheap.hpp>
#include <awn/mem/mem_gpuheapmanager.vk.hpp>
