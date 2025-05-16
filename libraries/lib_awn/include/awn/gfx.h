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

#include <awn/gfx/gfx_nngfxinteroperation.h>
#ifdef VP_TARGET_GRAPHICS_API_vk
    #include <awn/gfx/gfx_memorypool.vk.h>
#elifdef VP_TARGET_GRAPHICS_API_nvn
    #include <awn/gfx/gfx_memorypool.nvn.h>
#endif
#include <awn/gfx/gfx_queueapi.h>

#ifdef VP_TARGET_GRAPHICS_API_vk
    #include <awn/gfx/impl/gfx_proc.vk.h>
    #include <awn/gfx/gfx_features.vk.h>
    #include <awn/gfx/gfx_memoryapi.h>
    #include <awn/gfx/impl/gfx_resourceapi.vk.h>

    #include <awn/gfx/gfx_sync.vk.h>
    #include <awn/gfx/impl/gfx_context.vk.hpp>
    #include <awn/gfx/gfx_buffer.vk.h>
    #include <awn/gfx/gfx_texture.vk.h>
    #include <awn/gfx/gfx_rendertarget.vk.hpp>
    #include <awn/gfx/gfx_shader.vk.h>

#elifdef VP_TARGET_GRAPHICS_API_nvn
    #include <awn/gfx/impl/gfx_proc.nvn.h>
    #include <awn/gfx/gfx_memoryapi.h>
    #include <awn/gfx/impl/gfx_resourceapi.nvn.h>

    #include <awn/gfx/gfx_sync.nvn.h>
    #include <awn/gfx/impl/gfx_context.nvn.hpp>
    #include <awn/gfx/gfx_buffer.nvn.h>
    #include <awn/gfx/gfx_texture.nvn.h>
    #include <awn/gfx/gfx_rendertarget.nvn.h>
    #include <awn/gfx/gfx_shader.nvn.h>
#endif

#include <awn/gfx/impl/gfx_registertask.hpp>
#include <awn/gfx/gfx_texturebinder.h>
#include <awn/gfx/gfx_bufferbinder.h>
#include <awn/gfx/gfx_shaderarchivebinder.h>
#include <awn/gfx/impl/gfx_textureunit.h>
#include <awn/gfx/impl/gfx_bufferunit.h>
#include <awn/gfx/impl/gfx_shaderarchiveunit.h>
#include <awn/gfx/impl/gfx_asyncgpuresourcemanager.h>

#ifdef VP_TARGET_GRAPHICS_API_vk
    #include <awn/gfx/impl/gfx_textureviewsamplermanager.vk.h>
#elifdef VP_TARGET_GRAPHICS_API_nvn
    #include <awn/gfx/impl/gfx_textureviewsamplermanager.nvn.h>    
#endif

#include <awn/gfx/gfx_textureviewbinder.h>
#include <awn/gfx/gfx_samplerbinder.h>

#include <awn/gfx/gfx_pipelinestate.h>

#ifdef VP_TARGET_GRAPHICS_API_vk
    #include <awn/gfx/impl/gfx_descriptormemorymanager.vk.h>
    #include <awn/gfx/impl/gfx_commandpoolmanager.vk.h>
    #include <awn/gfx/gfx_commandbuffer.vk.hpp>
#elifdef VP_TARGET_GRAPHICS_API_nvn
    #include <awn/gfx/gfx_commandbuffer.nvn.hpp>
#endif

#include <awn/gfx/gfx_bufferallocator.hpp>

#include <awn/gfx/gfx_primitiveshape.h>
#include <awn/gfx/gfx_primitiveshapequad.h>
#include <awn/gfx/gfx_primitiveshapecircle.h>
