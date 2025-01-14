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

namespace awn::gfx {

    DECLARE_RESULT_MODULE(15);
    DECLARE_RESULT(MaximumGpuHeaps,                         1);
    DECLARE_RESULT(InvalidPriority,                         2);
    DECLARE_RESULT(NoBnsh,                                  3);
    DECLARE_RESULT(NoTextureInfo,                           4);
    DECLARE_RESULT(NoBufferInfo,                            5);
    DECLARE_RESULT(MipLevelCountMismatch,                   6);
    DECLARE_RESULT(MaxMipLevelCountExceeded,                7);
    DECLARE_RESULT(InvalidMemoryPropertyFlags,              8);
    DECLARE_RESULT(RegisterGuardSet,                        9);
    DECLARE_RESULT(InvalidReferenceBinder,                  10);
    DECLARE_RESULT(NoTexture,                               11);
    DECLARE_RESULT(InvalidTextureBinder,                    12);
    DECLARE_RESULT(TextureBinderAndTextureViewInfoMismatch, 13);
    DECLARE_RESULT(ExhaustedTextureSlots,                   14);
    DECLARE_RESULT(ExhaustedSamplerSlots,                   15);
    DECLARE_RESULT(InvalidDescriptorSlot,                   16);
    DECLARE_RESULT(InvalidShaderCodeType,                   17);
    DECLARE_RESULT(InitializedByDifferentProgram,           18);
    DECLARE_RESULT(NullArgument,                            19);
    DECLARE_RESULT(InvalidGpuMemoryHandle,                  20);
    DECLARE_RESULT(InvalidGpuAddress,                       21);

    DECLARE_RESULT(UnknownApiError,                         50);
    DECLARE_RESULT(NotReady,                                51);
    DECLARE_RESULT(Timeout,                                 52);
    DECLARE_RESULT(Incomplete,                              53);
    DECLARE_RESULT(OutOfCpuMemory,                          54);
    DECLARE_RESULT(OutOfGpuMemory,                          55);
    DECLARE_RESULT(GpuLost,                                 56);
    DECLARE_RESULT(FailedToMapMemory,                       57);
    DECLARE_RESULT(SubOptimal,                              58);
    DECLARE_RESULT(OutOfDate,                               59);
    DECLARE_RESULT(InvalidShaderCode,                       60);
}
