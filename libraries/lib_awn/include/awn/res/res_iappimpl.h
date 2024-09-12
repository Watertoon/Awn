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

	struct ExtensionInfo {
        MaxExtensionString  extension;
        CompressionType     compression_type;

        constexpr bool operator<(const ExtensionInfo &rhs) const {
            return extension < rhs.extension;
        }
        constexpr bool operator>(const ExtensionInfo &rhs) const {
            return extension > rhs.extension;
        }
    };

    class IAsyncResourceApplicationImpl {
        public:
            constexpr IAsyncResourceApplicationImpl() {/*...*/}
            virtual ~IAsyncResourceApplicationImpl() {/*...*/}
            
            virtual void Initialize(mem::Heap *heap) { VP_UNUSED(heap); }
            virtual u32 GetExtensionArrayCount() const { return 0; }
            virtual ExtensionInfo *GetExtensionArray() const { return nullptr; }
            
            virtual constexpr bool IsDefaultFileDevice() const { return false; }
            virtual FileDeviceBase *GetDefaultFileDevice(FileDeviceBase *base_device, const char *formatted_path, const char *original_path) { VP_UNUSED(base_device, formatted_path, original_path); return nullptr; }
    };
}
