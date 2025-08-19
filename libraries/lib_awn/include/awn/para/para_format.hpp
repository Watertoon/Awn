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

	constexpr const char *AdvanceNoWorkExtension(const char *query_path) {

		/* Remove work extension */
		const u8 current_char = *query_path;
		if (current_char == '?' || current_char == '/') {
			++query_path;
		} else if (vp::util::ToLowercase(current_char) == 'w' 
				&& vp::util::ToLowercase(query_path[1]) == 'o' 
				&& vp::util::ToLowercase(query_path[2]) == 'r' 
				&& vp::util::ToLowercase(query_path[3]) == 'k' 
				&& query_path[4] == '/') {
			query_path += 5;
		}

		return query_path;
	}

	constexpr void FormatNoWorkExtension(vp::util::MaxPathString *out_string, const char *query_path) {
		const char *adv_path = AdvanceNoWorkExtension(query_path);
        out_string->SetString(adv_path);
	}

	constexpr bool FormatSplitNameAndClass(const char **out_file_path, const char **out_class_name, vp::util::MaxPathString *split_string) {

		/* Walkback 2 '.' */
		constexpr char  delimiter      = '.';
		constexpr u32   walkback_max   = 2;
		u32             walkback_count = 0;
		char           *string         = split_string->GetString();
		for (u32 i = 0; i < split_string->GetLength(); --i) {

			if (string[i] != delimiter) { continue; }

			string[i] = '\0';

			++walkback_count;
			if (walkback_count != walkback_max) { continue; }

			*out_file_path  = string;
			*out_class_name = string + i + 1;

			return true;
		}

		return false;
	}

    constexpr bool FormatDirectoryUsingParentPath(vp::util::MaxPathString *out_path, const char *parent_path, const char *file_path) {

        /* Get directory of parent path */
        vp::util::GetDirectory(out_path, parent_path);

        /* Copy output directories */
        u32 iter     = 0;
        u32 end_iter = vp::util::StringLength(file_path);
        char copy_buffer[0x80] = {};
        while (iter < end_iter) {

            /* Pick out next directory sub string */
            u32 i = 0;
            for (; i < 0x80; ++i) {
                const char next_char = file_path[i + iter];
                if (next_char != '\0' && next_char != '/') { copy_buffer[i] = next_char; continue; }
                copy_buffer[i] = '\0';
                iter = iter + i + 1;
                break;
            }

            /* Handle . */
            if (copy_buffer[0] == '.') {
                
                /* Continue if . directory */
                if (copy_buffer[1] == '\0') { continue; }

                /* Remove directory for .. */
                if (copy_buffer[1] == '.' && copy_buffer[1]) {
                    vp::util::GetDirectory(out_path, out_path);
                }
            }

            /* Append directory to file path */
            if (out_path->IsEmpty()) {
                out_path->SetString(copy_buffer, i);
            } else {
                out_path->AppendFormat("\%s", copy_buffer);
            }
        }

        return true;
    }

    constexpr ALWAYS_INLINE u8 ParseHexChar(u8 character) {
        /* Check character is [, ], {, or } */
        const u8 number_char = character - '0';
        if (number_char < 10) { return number_char; }
        if (character - 'a' < 6) { return character - 'W'; }
        return (5 < (character - 'A')) ? character - '7' : 0;
    }

    constexpr ALWAYS_INLINE bool IsBracketCharacter(u8 character) {
        /* Check character is [, ], {, or } */
        const u8 adjusted_char = character - '[';
        return (adjusted_char < '#') & (((1ull << (adjusted_char & 0x3f)) & 0x5'0000'0005) != 0);
    }

    constexpr bool AdvanceCompositeSearchString(const char **out_next, const char *search_string, const char *name) {

        u32 format_result       = 0;
        u8  current_char        = *search_string;
        u8  last_char           = 0;
        const char *search_iter = search_string + 1;
        const char *name_iter   = name;
        if (current_char == '\0') { *out_next = ""; return true; }
        do {

            /* Handle byte code via % format */
            if (current_char == '%') {
                format_result = 1;

                ++search_iter;
                current_char = *search_iter;
                continue;
            } else if (format_result == 1) {
                format_result = 2;
                last_char = current_char;

                ++search_iter;
                current_char = *search_iter;
                continue;
            } else if (format_result == 2) {
                current_char  = (ParseHexChar(last_char) << 4) | ParseHexChar(current_char);
                format_result = 0;
            }

            /* Check against name */
            if (*name_iter != current_char) { return false; }

            /* Advance search iter */
            ++search_iter;
            current_char = *search_iter;

            /* Advance name iter */
            ++name_iter;
            if (*name_iter == '\0') { break; }
        } while (current_char != '\0');

        /* Fail if not a period or bracket character  */
        if (current_char != '.' && IsBracketCharacter(current_char) == false) { return false; }
        /* Advance past period or bracket */
        do {
            ++search_iter;
            current_char = *search_iter;
            if (current_char == '\0') {search_iter = ""; break; }
        } while (current_char == '.' || IsBracketCharacter(current_char) == true);

        *out_next = search_iter;

        return true;
    }
}
