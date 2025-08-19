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

    constexpr inline size_t cMaxPath = 260;

#ifdef VP_TARGET_PLATFORM_win32
    static_assert(cMaxPath == MAX_PATH);
#endif

    constexpr inline size_t cMaxDrive = 32;
    constexpr inline size_t cMaxExtension = 16;

    using MaxPathString      = vp::util::FixedString8<cMaxPath>;
    using MaxDriveString     = vp::util::FixedString8<cMaxDrive>;
    using MaxExtensionString = vp::util::FixedString8<cMaxExtension>;

    static constexpr u32 cInvalidDelimiter = 0xffff'ffff;

    template <typename T>
        requires (IsCharRaw<T>)
    constexpr T ToLowercase(T character) {
        return (0x19 < character - 0x41) ? character : character + 0x20;   
    }

    template <typename T>
        requires (IsStringRaw<T>)
    constexpr u32 FindDelimiter(const T *path, UnderlyingCharType<T> delimiter) {

        /* Find delimiter */
        u32 drive_offset = 0;
        {
            do {
                if (path[drive_offset] == delimiter) { break; }
                if (path[drive_offset] == static_cast<UnderlyingCharType<T>>('\0')) { return cInvalidDelimiter; }
                ++drive_offset;
            } while (drive_offset != 0);
        }

        return drive_offset;
    }

    template <typename T>
        requires (IsStringRaw<T>)
    constexpr u32 FindDriveDelimiter(const T *path, u32 path_len) {

        /* Find ':' */
        u32 drive_offset = path_len - 1;
        {
            do {
                if (path[drive_offset] == static_cast<UnderlyingCharType<T>>(':')) { break; }
                --drive_offset;
            } while (drive_offset != 0);
        }

        return drive_offset;
    }

    template <typename T>
        requires (IsStringRaw<T>)
    constexpr u32 FindDirectoryDelimiter(const T *path, u32 path_len) {
        
        /* Find '\\' */
        u32 back_slash_offset = path_len - 1;
        {
            do {
                if (path[back_slash_offset] == static_cast<UnderlyingCharType<T>>('\\')) { break; }
                --back_slash_offset;
            } while (back_slash_offset != 0);
        }

        /* Find '/' */
        u32 forward_slash_offset = path_len - 1;
        {
            do {
                if (path[forward_slash_offset] == static_cast<UnderlyingCharType<T>>('/')) { break; }
                --forward_slash_offset;
            } while (forward_slash_offset != 0);
        }
        
        return (forward_slash_offset < back_slash_offset) ? back_slash_offset : forward_slash_offset;
    }

    template <typename T>
        requires (IsStringRaw<T>)
    constexpr u32 FindDirectoryDelimiterForward(const T *path, u32 path_len) {

        /* Find '\\' */
        u32 back_slash_offset = 0;
        {
            do {
                if (path[back_slash_offset] == static_cast<UnderlyingCharType<T>>('\\')) { break; }
                ++back_slash_offset;
            } while (back_slash_offset != path_len);
        }

        /* Find '/' */
        u32 forward_slash_offset = 0;
        {
            do {
                if (path[forward_slash_offset] == static_cast<UnderlyingCharType<T>>('/')) { break; }
                ++forward_slash_offset;
            } while (forward_slash_offset != path_len);
        }
        const u32 offset = (back_slash_offset < forward_slash_offset) ? back_slash_offset : forward_slash_offset;
        return offset;
    }

    template <typename T>
        requires (IsStringRaw<T>)
    constexpr u32 FindExtensionDelimiter(const T *path, u32 path_len) {

        /* Find '.' */
        u32 dot_offset = path_len - 1;
        {
            do {
                if (path[dot_offset] == static_cast<UnderlyingCharType<T>>('.')) { break; }
                --dot_offset;
            } while (dot_offset != 0);
        }

        return dot_offset;
    }

    template <typename O, typename T>
        requires (IsFixedString<O>) && (IsFixedStringOrRaw<T>)
    constexpr void GetDrive(O *out_drive, T *path_with_drive) {

        /* Null check */
        if (out_drive == nullptr || path_with_drive == nullptr) { return; }

        /* Get string */
        const UnderlyingCharType<T> *path;
        u32 path_len;
        GetUnderlyingString(std::addressof(path), std::addressof(path_len), path_with_drive);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 drive_start_offset = FindDriveDelimiter(path, path_len);

         /* Copy string up to delimiter */
        out_drive->SetString(path, drive_start_offset);

        return;
    }

    template <typename O, typename T>
        requires (IsFixedString<O>) && (IsFixedStringOrRaw<T>)
    constexpr void GetPathWithoutDrive(O *out_path, T *path_with_drive) {

        /* Null check */
        if (out_path == nullptr || path_with_drive == nullptr) { return; }

        /* Get string */
        const UnderlyingCharType<T> *path;
        u32 path_len;
        GetUnderlyingString(std::addressof(path), std::addressof(path_len), path_with_drive);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 drive_base = FindDriveDelimiter(path, path_len);
        const u32 drive_start_offset = (drive_base != 0) ? drive_base + 2 : 0;

         /* Copy string from last delimiter */
        const u32 name_size = path_len - drive_start_offset;
        out_path->SetString(path + drive_start_offset, name_size);

        return;
    }

    template <typename O, typename T>
        requires (IsFixedString<O>) && (IsFixedStringOrRaw<T>)
    constexpr void GetPathWithoutExtension(O *out_path, T *path_with_extension) {

        /* Null check */
        if (out_path == nullptr || path_with_extension == nullptr) { return; }

        /* Get string */
        const UnderlyingCharType<T> *path;
        u32 path_len;
        GetUnderlyingString(std::addressof(path), std::addressof(path_len), path_with_extension);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 ext_start_offset = FindExtensionDelimiter(path, path_len);

         /* Copy string from last delimiter */
        const u32 name_size = ext_start_offset;
        out_path->SetString(path, name_size);

        return;
    }

    template <typename T>
        requires (IsStringRaw<T>)
    constexpr T *AdvancePathNoDirectory(T *path) {

        /* Null check */
        if (path == nullptr) { return path; }

        /* Strlen */
        const u32 path_len = vp::util::StringLength(path);

        /* Length check */
        if (path_len == 0) { return path; }

        /* Get file name offset */
        const u32 name_start_offset = FindDirectoryDelimiter(path, path_len) + 1;

        /* Advance string to cut off directory */
        return path + name_start_offset;
    }

    template <typename O, typename T>
        requires (IsFixedString<O>) && (IsFixedStringOrRaw<T>)
    constexpr void GetDirectory(O *out_directory, T *file_path) {

        /* Null check */
        if (out_directory == nullptr || file_path == nullptr) { return; }

        /* Get string */
        const UnderlyingCharType<T> *path;
        u32 path_len;
        GetUnderlyingString(std::addressof(path), std::addressof(path_len), file_path);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 name_start_offset = FindDirectoryDelimiter(path, path_len) + 1;

        /* Copy string until last delimiter */
        const u32 name_size = name_start_offset;
        out_directory->SetString(path, name_size);
        return;
    }

    template <typename O, typename T>
        requires (IsFixedString<O>) && (IsFixedStringOrRaw<T>)
    constexpr void GetDirectoryWithoutDrive(O *out_directory, T *path_with_drive) {

        /* Null check */
        if (out_directory == nullptr || path_with_drive == nullptr) { return; }

        /* Get string */
        const UnderlyingCharType<T> *path;
        u32 path_len;
        GetUnderlyingString(std::addressof(path), std::addressof(path_len), path_with_drive);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 dir_start_offset  = FindDriveDelimiter(path, path_len);
        const u32 name_start_offset = FindDirectoryDelimiter(path, path_len) + 1;

        /* Copy out directory */
        const u32 name_size = name_start_offset - dir_start_offset - name_start_offset;
        out_directory->SetString(path_with_drive + dir_start_offset, name_size);
        return;
    }

    template <typename O, typename T>
        requires (IsFixedString<O>) && (IsFixedStringOrRaw<T>)
    constexpr u32 GetUpToNextDirectoryFromBase(O *out_directory, T *directory_path, u32 base) {

        /* Null check */
        if (out_directory == nullptr || directory_path == nullptr) { return 0; }

        /* Get string */
        const UnderlyingCharType<T> *path;
        u32 path_len;
        GetUnderlyingString(std::addressof(path), std::addressof(path_len), directory_path);

        /* Length check */
        if (base == 0) {
            const u32 delimit = FindDriveDelimiter(path, path_len);
            if (delimit != 0) {
                base += delimit + 1;
            }
        }
        base = base + 1;
        if (path_len <= base) { return path_len; }

        /* Get file name offset */
        const UnderlyingCharType<T> *dir_offset = path + base;
        const u32 dir_off_len = path_len - base;
        const u32 dir_start_offset = FindDirectoryDelimiterForward(dir_offset, dir_off_len);

        /* Copy out directory */
        const u32 total_size = dir_start_offset + base;
        const u32 copy_size = (total_size >= path_len - 1) ? path_len : total_size;
        out_directory->SetString(path, copy_size);
        return copy_size;
    }

    template <typename O, typename T>
        requires (IsFixedString<O>) && (IsFixedStringOrRaw<T>)
    constexpr void GetFileNameWithExtension(O *out_file_name, T *file_path) {

        /* Null check */
        if (out_file_name == nullptr || file_path == nullptr) { return; }

        /* Get string */
        const UnderlyingCharType<T> *path;
        u32 path_len;
        GetUnderlyingString(std::addressof(path), std::addressof(path_len), file_path);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 name_start_offset = FindDirectoryDelimiter(path, path_len) + 1;

        /* Copy string from last delimiter */
        const u32 name_size = path_len - name_start_offset;
        out_file_name->SetString(path + name_start_offset, name_size);
        return;
    }

    template <typename O, typename T>
        requires (IsFixedString<O>) && (IsFixedStringOrRaw<T>)
    constexpr void GetFileNameWithoutExtension(O *out_file_name, T *path_with_extension) {

        /* Null check */
        if (out_file_name == nullptr || path_with_extension == nullptr) { return; }

        /* Get string */
        const UnderlyingCharType<T> *path;
        u32 path_len;
        GetUnderlyingString(std::addressof(path), std::addressof(path_len), path_with_extension);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        const u32 ext_start_offset  = FindExtensionDelimiter(path, path_len);
        const u32 name_start_offset = FindDirectoryDelimiter(path, path_len) + 1;

        /* Copy string from last delimiter */
        const u32 name_size = ext_start_offset - name_start_offset;
        out_file_name->SetString(path + name_start_offset, name_size);
        return;
    }

    template <typename O, typename T>
        requires (IsFixedString<O>) && (IsFixedStringOrRaw<T>)
    constexpr void GetExtension(O *out_extension, T *file_path) {

       /* Null check */
        if (out_extension == nullptr || file_path == nullptr) { return; }

        /* Get string */
        const UnderlyingCharType<T> *path;
        u32 path_len;
        GetUnderlyingString(std::addressof(path), std::addressof(path_len), file_path);

        /* Length check */
        if (path_len == 0) { return; }

        /* Get extension offset */
        const u32 ext_start_offset = FindExtensionDelimiter(path, path_len);

        /* Copy string until last delimiter */
        const u32 ext_len  = (path_len - ext_start_offset);
        const u32 ext_size = (ext_len < cMaxExtension) ? ext_len : (cMaxExtension - 1);
        out_extension->SetString(path + ext_start_offset, ext_size);
        return;
    }

    template <typename T>
        requires (IsFixedString<T>)
    constexpr void ChangeDirectoryDelimiter(T *in_out_path, UnderlyingCharType<T> new_delimiter) {

        /* Null check */
        if (in_out_path == nullptr) { return; }

        /* strlen */
        const u32 path_len = in_out_path->GetLength();

        /* Length check */
        if (path_len == 0) { return; }

        /* Get file name offset */
        u32 start_offset = FindDirectoryDelimiter(in_out_path->m_string_array, path_len) + 1;

        while (start_offset != 0) {
            if (in_out_path->m_string_array[start_offset] == '/' || in_out_path->m_string_array[start_offset] == '\\') { in_out_path->m_string_array[start_offset] = new_delimiter; }
            --start_offset;
        }
        return;
    }
}
