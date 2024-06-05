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

namespace awn::para {

	class DummyFileDevice : public res::FileDeviceBase {
        public:
            static constexpr u8 cDummyByaml[] = {
                'Y','B',0x07,0x00,
                0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,
                0x10,0x00,0x00,0x00,
                0xC1,0x00,0x00,0x00,
            };
            static constexpr u32 cDummyByamlSize = sizeof(cDummyByaml);
		public:
            constexpr DummyFileDevice() : FileDeviceBase() {/*...*/}
            virtual ~DummyFileDevice() override {/*...*/}

            virtual Result OpenFileImpl(res::FileHandle *out_file_handle, const char *path, res::OpenMode open_mode) override {
                VP_UNUSED(out_file_handle, path);

                /* Integrity checks */
                RESULT_RETURN_IF(open_mode != res::OpenMode::Read, res::ResultInvalidOpenMode);

                RESULT_RETURN_SUCCESS;
            }
            virtual Result CloseFileImpl(res::FileHandle *file_handle) override {
                VP_UNUSED(file_handle);
                RESULT_RETURN_SUCCESS;
            }
            virtual Result ReadFileImpl(void *read_buffer, size_t *out_read_size, res::FileHandle *file_handle, size_t read_size, size_t file_offset) override {
                VP_UNUSED(file_handle);

                /* Integrity checks */
                RESULT_RETURN_IF(cDummyByamlSize < file_offset, res::ResultInvalidFileOffset);

                const size_t size_to_read = vp::util::Min(cDummyByamlSize - file_offset, read_size);
                if (out_read_size != nullptr) {
                    *out_read_size = size_to_read;
                }

                ::memcpy(read_buffer, cDummyByaml + file_offset, size_to_read);

                RESULT_RETURN_SUCCESS;
            }

            virtual Result GetFileSizeImpl(size_t *out_size, res::FileHandle *file_handle) override {
                VP_UNUSED(file_handle);
                *out_size = cDummyByamlSize;
                RESULT_RETURN_SUCCESS;
            }
            virtual Result GetFileSizeImpl(size_t *out_size, const char *path) override {
                VP_UNUSED(path);
                *out_size = cDummyByamlSize;
                RESULT_RETURN_SUCCESS;
            }

            virtual Result CheckFileExistsImpl(const char *path) override { 
                VP_UNUSED(path);
                RESULT_RETURN_SUCCESS; 
            }
	};
}
