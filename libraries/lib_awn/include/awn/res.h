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

#include <awn/res/res_idecompressor.h>
#include <awn/res/res_icompressor.h>
#include <awn/res/res_filedevicebase.h>

#ifdef VP_TARGET_PLATFORM_win32
    #include <awn/res/res_systemfiledevice.win32.h>
#elifdef VP_TARGET_PLATFORM_nx
    #include <awn/res/res_systemfiledevice.nx.h>
#elif VP_TARGET_PLATFORM_linux
    #include <awn/res/res_systemfiledevice.linux.h>
#endif

#include <awn/res/impl/res_filedevicemanager.h>
#include <awn/res/res_parallelzstddecompressor.h>
#include <awn/res/res_resource.h>
#include <awn/res/res_archiveresource.h>
#include <awn/res/res_archivefiledevice.h>
#include <awn/res/res_resourcefactorybase.h>
#include <awn/res/impl/res_resourcefactorymanager.h>
#include <awn/res/res_fileapi.h>
#include <awn/res/res_resourceapi.h>

#include <awn/res/res_sarcarchiveresource.h>
#include <awn/res/res_sarcresourcefactory.h>
#include <awn/res/res_beaarchiveresource.h>

#include <awn/res/impl/res_asyncsavemanager.h>

#include <awn/res/res_iappimpl.h>
#include <awn/res/impl/res_resourcesizetablemanager.h>
#include <awn/res/impl/res_extensionmanager.h>
#include <awn/res/res_resourcebinder.h>
#include <awn/res/impl/res_resourceunit.h>
#include <awn/res/res_asyncresourceapi.h>
#include <awn/res/impl/res_resourceunitmanager.h>
#include <awn/res/impl/res_decompressormanager.h>
#include <awn/res/impl/res_resourcememorymanager.h>
#include <awn/res/impl/res_resourceunitallocator.h>
#include <awn/res/impl/res_loadtask.hpp>
#include <awn/res/impl/res_threadlocalarchivemanager.h>
#include <awn/res/impl/res_resourcesizetablemanager.h>
#include <awn/res/impl/res_asyncresourcemanager.h>
