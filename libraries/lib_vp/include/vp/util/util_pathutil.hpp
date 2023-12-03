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

namespace vp::util {

    constexpr u32 FindDriveDelimiter(const char *path, u32 path_len) {

        /* Find ':' */
        u32 drive_offset = path_len;
        {
            do {
                if (path[drive_offset] == ':') { break; }
                --drive_offset;
            } while (drive_offset != 0);
        }

        return drive_offset;
    }

    constexpr u32 FindDirectoryDelimiter(const char *path, u32 path_len) {
        
        /* Find '\\' */
        u32 back_slash_offset = path_len;
        {
            do {
                if (path[back_slash_offset] == '\\') { break; }
                --back_slash_offset;
            } while (back_slash_offset != 0);
        }

        /* Find '/' */
        u32 forward_slash_offset = path_len;
        {
            do {
                if (path[forward_slash_offset] == '/') { break; }
                --forward_slash_offset;
            } while (forward_slash_offset != 0);
        }
        
        return (forward_slash_offset < back_slash_offset) ? back_slash_offset : forward_slash_offset;
    }

    constexpr u32 FindExtensionDelimiter(const char *path, u32 path_len) {

        /* Find '.' */
        u32 dot_offset = path_len;
        {
            do {
                if (path[dot_offset] == '.') { break; }
                --dot_offset;
            } while (dot_offset != 0);
        }

        return dot_offset;
    }

    constexpr inline size_t cMaxPath      = 260;
    constexpr inline size_t cMaxDrive     = 32;
    constexpr inline size_t cMaxExtension = 16;

    using MaxPathString = vp::util::FixedString<cMaxPath>;
    using MaxDriveString = vp::util::FixedString<cMaxDrive>;
    using MaxExtensionString = vp::util::FixedString<cMaxExtension>;

    constexpr void GetDrive(MaxDriveString *out_drive, const char *path_with_drive) {

        /* Null check */
        if (out_drive == nullptr || path_with_drive == nullptr) { return; }

        /* strlen */
        const u32 path_len = (std::is_constant_evaluated() == true) ? TStringLength(path_with_drive, cMaxPath + 1) : ::strlen(path_with_drive);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 drive_start_offset = FindDriveDelimiter(path_with_drive, path_len);

         /* Copy string from last delimiter */
        const u32 name_size = drive_start_offset;
        ::memcpy(out_drive->m_string_array, path_with_drive, (name_size < cMaxDrive) ? name_size : (cMaxDrive - 1));
        out_drive->m_length = name_size;

        out_drive->AssureTermination();
    }

    constexpr void GetDrive(MaxDriveString *out_drive, MaxPathString *path_with_drive) {

        /* Null check */
        if (out_drive == nullptr || path_with_drive == nullptr) { return; }

        /* strlen */
        const u32 path_len = path_with_drive->m_length;

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 drive_start_offset = FindDriveDelimiter(path_with_drive->m_string_array, path_len);

         /* Copy string from last delimiter */
        const u32 name_size = drive_start_offset;
        ::memcpy(out_drive->m_string_array, path_with_drive->m_string_array, (name_size < cMaxDrive) ? name_size : (cMaxDrive - 1));
        out_drive->m_length = name_size;

        out_drive->AssureTermination();
    }

    constexpr void GetPathWithoutDrive(MaxPathString *out_path, const char *path_with_drive) {

        /* Null check */
        if (out_path == nullptr || path_with_drive == nullptr) { return; }

        /* strlen */
        const u32 path_len = (std::is_constant_evaluated() == true) ? TStringLength(path_with_drive, cMaxPath + 1) : ::strlen(path_with_drive);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 drive_start_offset = FindDriveDelimiter(path_with_drive, path_len);

         /* Copy string from last delimiter */
        const u32 name_size = path_len - drive_start_offset;
        ::memcpy(out_path->m_string_array, path_with_drive + drive_start_offset, (name_size < cMaxPath) ? name_size : (cMaxPath - 1));
        out_path->m_length = name_size;

        out_path->AssureTermination();
    }

    constexpr void GetPathWithoutDrive(MaxPathString *out_path, MaxPathString *path_with_drive) {

        /* Null check */
        if (out_path == nullptr || path_with_drive == nullptr) { return; }

        /* strlen */
        const u32 path_len = path_with_drive->m_length;

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 drive_start_offset = FindDriveDelimiter(path_with_drive->m_string_array, path_len);

         /* Copy string from last delimiter */
        const u32 name_size = path_len - drive_start_offset;
        ::memcpy(out_path->m_string_array, path_with_drive->m_string_array + drive_start_offset, (name_size < cMaxPath) ? name_size : (cMaxPath - 1));
        out_path->m_length = name_size;
    
        out_path->AssureTermination();
    }

    constexpr void GetPathWithoutExtension(MaxPathString *out_path, const char *path_with_extension) {

        /* Null check */
        if (out_path == nullptr || path_with_extension == nullptr) { return; }

        /* strlen */
        const u32 path_len = (std::is_constant_evaluated() == true) ? TStringLength(path_with_extension, cMaxPath + 1) : ::strlen(path_with_extension);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 ext_start_offset = FindExtensionDelimiter(path_with_extension, path_len);

         /* Copy string from last delimiter */
        const u32 name_size = ext_start_offset;
        ::memcpy(out_path->m_string_array, path_with_extension, name_size);
        out_path->m_length = name_size;

        out_path->AssureTermination();
    }

    constexpr void GetPathWithoutExtension(MaxPathString *out_path, MaxPathString *path_with_extension) {

        /* Null check */
        if (out_path == nullptr || path_with_extension == nullptr) { return; }

        /* strlen */
        const u32 path_len = path_with_extension->m_length;

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 ext_start_offset = FindExtensionDelimiter(path_with_extension->m_string_array, path_len);

         /* Copy string from last delimiter */
        const u32 name_size = ext_start_offset;
        ::memcpy(out_path->m_string_array, path_with_extension, name_size);
        out_path->m_length = name_size;
    
        out_path->AssureTermination();
    }

    constexpr void GetDirectory(MaxPathString *out_directory, const char *path) {

        /* Null check */
        if (out_directory == nullptr || path == nullptr) { return; }

        /* strlen */
        const u32 path_len = (std::is_constant_evaluated() == true) ? TStringLength(path, cMaxPath + 1) : ::strlen(path);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 name_start_offset = FindDirectoryDelimiter(path, path_len) + 1;

        /* Copy string until last delimiter */
        const u32 name_size = name_start_offset;
        ::memcpy(out_directory->m_string_array, path, (name_size < cMaxPath) ? name_size : (cMaxPath - 1));
        out_directory->m_length = name_size;

        out_directory->AssureTermination();
    }

    constexpr void GetDirectory(MaxPathString *out_directory, MaxPathString *path) {

        /* Null check */
        if (out_directory == nullptr || path == nullptr) { return; }

        /* strlen */
        const u32 path_len = path->m_length;

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 name_start_offset = FindDirectoryDelimiter(path->m_string_array, path_len) + 1;

        /* Copy string until last delimiter */
        const u32 name_size = name_start_offset;
        ::memcpy(out_directory->m_string_array, path->m_string_array, (name_size < cMaxPath) ? name_size : (cMaxPath - 1));
        out_directory->m_length = name_size;

        out_directory->AssureTermination();
    }

    constexpr void GetDirectoryWithoutDrive(MaxPathString *out_directory, const char *path_with_drive) {

        /* Null check */
        if (out_directory == nullptr || path_with_drive == nullptr) { return; }

        /* strlen */
        const u32 path_len = (std::is_constant_evaluated() == true) ? TStringLength(path_with_drive, cMaxPath + 1) : ::strlen(path_with_drive);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 dir_start_offset  = FindDriveDelimiter(path_with_drive, path_len);
        const u32 name_start_offset = FindDirectoryDelimiter(path_with_drive, path_len) + 1;

        /* Copy out directory */
        const u32 name_size = name_start_offset - dir_start_offset - name_start_offset;
        ::memcpy(out_directory->m_string_array, path_with_drive + dir_start_offset, (name_size < cMaxPath) ? name_size : (cMaxPath - 1));
        out_directory->m_length = name_size;

        out_directory->AssureTermination();
    }

    constexpr void GetDirectoryWithoutDrive(MaxPathString *out_directory, MaxPathString *path_with_drive) {

        /* Null check */
        if (out_directory == nullptr || path_with_drive == nullptr) { return; }

        /* strlen */
        const u32 path_len = path_with_drive->m_length;

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 dir_start_offset  = FindDriveDelimiter(path_with_drive->m_string_array, path_len);
        const u32 name_start_offset = FindDirectoryDelimiter(path_with_drive->m_string_array, path_len) + 1;

        /* Copy out directory */
        const u32 name_size = name_start_offset - dir_start_offset - name_start_offset;
        ::memcpy(out_directory->m_string_array, path_with_drive->m_string_array + dir_start_offset, (name_size < cMaxPath) ? name_size : (cMaxPath - 1));
        out_directory->m_length = name_size;

        out_directory->AssureTermination();
    }

    constexpr void GetFileNameWithExtension(MaxPathString *out_file_name, const char *path) {

        /* Null check */
        if (out_file_name == nullptr || path == nullptr) { return; }

        /* strlen */
        const u32 path_len = (std::is_constant_evaluated() == true) ? TStringLength(path, cMaxPath + 1) : ::strlen(path);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 name_start_offset = FindDirectoryDelimiter(path, path_len) + 1;

        /* Copy string from last delimiter */
        const u32 name_size = path_len - name_start_offset;
        ::memcpy(out_file_name->m_string_array, path + name_start_offset, (name_size < cMaxPath) ? name_size : (cMaxPath - 1));
        out_file_name->m_length = name_size;

        out_file_name->AssureTermination();
    }

    constexpr void GetFileNameWithExtension(MaxPathString *out_file_name, MaxPathString *path) {

        /* Null check */
        if (out_file_name == nullptr || path == nullptr) { return; }

        /* strlen */
        const u32 path_len = path->m_length;

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 name_start_offset = FindDirectoryDelimiter(path->m_string_array, path_len) + 1;

        /* Copy string from last delimiter */
        const u32 name_size = path_len - name_start_offset;
        ::memcpy(out_file_name->m_string_array, path->m_string_array + name_start_offset, (name_size < cMaxPath) ? name_size : (cMaxPath - 1));
        out_file_name->m_length = name_size;

        out_file_name->AssureTermination();
    }

    constexpr void GetFileNameWithoutExtension(MaxPathString *out_file_name, const char *path_with_extension) {

        /* Null check */
        if (out_file_name == nullptr || path_with_extension == nullptr) { return; }

        /* strlen */
        const u32 path_len = (std::is_constant_evaluated() == true) ? TStringLength(path_with_extension, cMaxPath + 1) : ::strlen(path_with_extension);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 ext_start_offset  = FindExtensionDelimiter(path_with_extension, path_len);
        const u32 name_start_offset = FindDirectoryDelimiter(path_with_extension, path_len) + 1;

        /* Copy string from last delimiter */
        const u32 name_size = ext_start_offset - name_start_offset;
        ::memcpy(out_file_name->m_string_array, path_with_extension + name_start_offset, (name_size < cMaxPath) ? name_size : (cMaxPath - 1));
        out_file_name->m_length = name_size;

        out_file_name->AssureTermination();
    }

    constexpr void GetFileNameWithoutExtension(MaxPathString *out_file_name, MaxPathString *path_with_extension) {

        /* Null check */
        if (out_file_name == nullptr || path_with_extension == nullptr) { return; }

        /* strlen */
        const u32 path_len = path_with_extension->m_length;

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 ext_start_offset  = FindExtensionDelimiter(path_with_extension->m_string_array, path_len);
        const u32 name_start_offset = FindDirectoryDelimiter(path_with_extension->m_string_array, path_len) + 1;

        /* Copy string from last delimiter */
        const u32 name_size = ext_start_offset - name_start_offset;
        ::memcpy(out_file_name->m_string_array, path_with_extension->m_string_array + name_start_offset, (name_size < cMaxPath) ? name_size : (cMaxPath - 1));
        out_file_name->m_length = name_size;

        out_file_name->AssureTermination();
    }

    constexpr void GetExtension(MaxExtensionString *out_extension, const char *path) {

       /* Null check */
        if (out_extension == nullptr || path == nullptr) { return; }

        /* strlen */
        const u32 path_len = (std::is_constant_evaluated() == true) ? TStringLength(path, cMaxPath + 1) : ::strlen(path);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get extension offset */
        const u32 ext_start_offset = FindExtensionDelimiter(path, path_len);

        /* Copy string until last delimiter */
        const u32 ext_len  = (path_len - ext_start_offset);
        const u32 ext_size = (ext_len < cMaxExtension) ? ext_len : (cMaxExtension - 1);
        ::memcpy(out_extension->m_string_array, path + ext_start_offset, ext_size);
        out_extension->m_length = ext_size;

        out_extension->AssureTermination();
    }

    constexpr void GetExtension(MaxExtensionString *out_extension, MaxPathString *path) {

       /* Null check */
        if (out_extension == nullptr || path == nullptr) { return; }

        /* strlen */
        const u32 path_len = path->m_length;

        /* Length check */
        if (path_len == 0) { return; }

        /* Get extension offset */
        const u32 ext_start_offset = FindExtensionDelimiter(path->m_string_array, path_len);

        /* Copy string until last delimiter */
        const u32 ext_len  = (path_len - ext_start_offset);
        const u32 ext_size = (ext_len < cMaxExtension) ? ext_len : (cMaxExtension - 1);
        ::memcpy(out_extension->m_string_array, path->m_string_array + ext_start_offset, ext_size);
        out_extension->m_length = ext_size;

        out_extension->AssureTermination();
    }

    constexpr void ChangeDirectoryDelimiter(MaxPathString *in_out_path, char new_delimiter) {

        /* Null check */
        if (in_out_path == nullptr) { return; }

        /* strlen */
        const u32 path_len = in_out_path->m_length;

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        u32 start_offset = FindDirectoryDelimiter(in_out_path->m_string_array, path_len) + 1;

        while (start_offset != 0) {
            if (in_out_path->m_string_array[start_offset] == '/' || in_out_path->m_string_array[start_offset] == '\\') { in_out_path->m_string_array[start_offset] = new_delimiter; }
            --start_offset;
        }
    }
}
