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

namespace vp::res {

    struct ResBffnt : public ResUi2dHeader {
        static constexpr inline u32 Magic = util::TCharCode32("FFNT");
    };
    static_assert(sizeof(ResBffnt) == 0x14);

    enum class BffntImageFormat : u8 {
        R8G8B8A8_Unorm = 0x0,
        R8G8B8_Unorm   = 0x1,
        R5G5B5A1_Unorm = 0x2,
        R5G6B5_Unorm   = 0x3,
        R4G4B4A4_Unorm = 0x4,
        LA8_Unorm      = 0x5,
        LA4_Unorm      = 0x6,
        A4_Unorm       = 0x7,
        A8_Unorm       = 0x8,
        BC1_Unorm      = 0x9,
        BC2_Unorm      = 0xa,
        BC3_Unorm      = 0xb,
        BC4_Unorm      = 0xc,
        BC5_Unorm      = 0xd,
        R8G8B8A8_SRGB  = 0xe,
        BC1_SRGB       = 0xf,
        BC2_SRGB       = 0x10,
        BC3_SRGB       = 0x11,
        BC7_Unorm      = 0x12,
        BC7_SRGB       = 0x13,
    };

    struct ResBffntTextureGlyph : public ResUi2dSection {
        u8  cell_width;
        u8  cell_height;
        u8  texture_count;
        u8  max_character_width;
        u32 per_texture_size;
        u16 baseline_position;
        u16 texture_format;
        u16 cells_per_row;
        u16 cells_per_column;
        u16 image_width;
        u16 image_height;
        u32 image_data_offset;

        ALWAYS_INLINE ResBntx *GetImageData(void *file_head) {
            return reinterpret_cast<ResBntx*>(reinterpret_cast<uintptr_t>(file_head) + image_data_offset);
        }

        static constexpr inline u32 cMagic = util::TCharCode32("TGLP");
    };
    static_assert(sizeof(ResBffntTextureGlyph) == 0x20);

    struct ResBffntCharacterWidthData {
        s8 left_width;
        u8 glyph_width;
        u8 char_width;
    };
    static_assert(sizeof(ResBffntCharacterWidthData) == 0x3);

    struct ResBffntCharacterWidths : public ResUi2dSection {
        u16                        first_entry_index;
        u16                        last_entry_index;
        u32                        next_character_width_offset;
        ResBffntCharacterWidthData width_data_array[];

        static constexpr inline u32 cMagic = util::TCharCode32("CWDH");
    };
    static_assert(sizeof(ResBffntCharacterWidths) == 0x10);

    enum class BffntMapMethod {
        Direct = 0x0,
        Table  = 0x1,
        Scan   = 0x2,
    };

    struct ResBffntScanEntry {
        u16 key;
        u16 code;
    };

    struct ResBffntScan {
        u16               entry_half_count;
        u16               reserve0;
        ResBffntScanEntry entry_array[];
    };

    struct ResBffntCodeMap : public ResUi2dSection {
        u32 code_range_begin;
        u32 code_range_end;
        u16 map_method;
        u16 reserve0;
        u32 next_code_map_offset;
        u16 code_map_array[];

        static constexpr inline u32 cMagic = util::TCharCode32("CMAP");

        ALWAYS_INLINE ResBffntCodeMap *GetNextCodeMap(void *file_base) const {
            return reinterpret_cast<ResBffntCodeMap*>(reinterpret_cast<uintptr_t>(file_base) + next_code_map_offset + sizeof(ResUi2dSection));
        }

        constexpr u16 MapCharacter(u32 key_code) const {

            /* Map code directly, via a 1 to 1 table, or via scanning a sorted array*/
            if (map_method == static_cast<u16>(BffntMapMethod::Direct)) { return code_map_array[0] + (key_code - code_range_begin); }
            if (map_method == static_cast<u16>(BffntMapMethod::Table)) { return code_map_array[(key_code - code_range_begin)]; }
            if (map_method != static_cast<u16>(BffntMapMethod::Scan)) { return 0xffff; }

            /* Binary search for key code */
            const u16 *start_iter = std::addressof(code_map_array[2]);
            const u16 *end_iter   = code_map_array + (code_map_array[0] * 2);
            while (start_iter <= end_iter) {

                /* Find next index */
                const u16 *pair = nullptr;
                for (;;) {
                    
                    const u32 offset = (reinterpret_cast<uintptr_t>(end_iter) - reinterpret_cast<uintptr_t>(start_iter)) / sizeof(size_t);
                    const u32 next_index = (offset == 0xffff'ffff) ?  0 : static_cast<u32>(offset);
                    pair = start_iter + (next_index) * 2;
                    if (key_code <= pair[0]) { break; }
                    
                    /* Advance iterator */
                    start_iter = pair + 2;
                    if (end_iter < start_iter) { return 0xffff; }
                }

                /* Check valid range */
                if (pair[0] < key_code || pair[0] == key_code) { return pair[1]; }

                /* Advance iterator */
                end_iter = pair - 2;
            }

            return 0xffff;
        }
    };
    static_assert(sizeof(ResBffntCodeMap) == 0x18);

    enum class BffntFontType : u8 {
        Glyph         = 0x0,
        Texture       = 0x1,
        PackedTexture = 0x2
    };
    enum class BffntCharacterEncoding : u8 {
        Unicode  = 0x0,
        ShiftJis = 0x1,
        CP1252   = 0x2
    };

    struct ResBffntFontInfo : public ResUi2dSection {
        u8                         font_type;
        u8                         height;
        u8                         width;
        u8                         ascent;
        u16                        line_feed;
        u16                        alternate_char_index;
        ResBffntCharacterWidthData default_character_widths;
        u8                         character_encoding;
        u32                        texture_glyph_section_offset;
        u32                        character_width_section_offset;
        u32                        code_map_section_offset;

        ResBffntTextureGlyph *GetTextureGlyph(void *file_base) {
            return (texture_glyph_section_offset == 0) ? 0 : reinterpret_cast<ResBffntTextureGlyph*>(reinterpret_cast<uintptr_t>(file_base) + texture_glyph_section_offset - 0x8);
        }
        ResBffntCharacterWidths *GetCharacterWidths(void *file_base) {
            return (character_width_section_offset == 0) ? 0 : reinterpret_cast<ResBffntCharacterWidths*>(reinterpret_cast<uintptr_t>(file_base) + character_width_section_offset - 0x8);
        }

        ResBffntCodeMap *GetCodeMapByCharacter(void *file_base, u32 key_code) const {

            /* Search code map list */
            u32 next_code_map_offset = code_map_section_offset - 0x8;
            while (next_code_map_offset != 0) {

                /* Get code map */
                ResBffntCodeMap *code_map = reinterpret_cast<ResBffntCodeMap*>(reinterpret_cast<uintptr_t>(file_base) + code_map_section_offset);

                /* Check code range */
                if (code_map->code_range_begin <= key_code && key_code <= code_map->code_range_end) { return code_map; }

                /* Get next code map offset */
                next_code_map_offset = code_map->next_code_map_offset;
            }

            return nullptr;
        }

        static constexpr inline u32 cMagic = util::TCharCode32("FINF");
    };
    static_assert(sizeof(ResBffntFontInfo) == 0x20);

    struct ResBffntKerningPair {
        u32 other_character;
        u32 kerning;
    };
    static_assert(sizeof(ResBffntKerningPair) == 0x8);

    struct ResBffntCharacterKerning {
        u16                 kerning_pair_count;
        u16                 reserve0;
        ResBffntKerningPair kerning_pair_array[];
    };
    static_assert(sizeof(ResBffntCharacterKerning) == 0x4);

    struct ResBffntKerningOffset {
        u32 character;
        u32 offset;
    };
    static_assert(sizeof(ResBffntKerningOffset) == 0x8);

    struct ResBffntKerningTable : public ResUi2dSection {
        u16                   kerning_offset_count;
        u16                   reserve0;
        ResBffntKerningOffset kerning_array[];

        static constexpr inline u32 cMagic = util::TCharCode32("KRNG");

        constexpr ResBffntKerningOffset *TryGetKerningByCharacter(u32 character) {

            /* Binary search pattern for character kerning */
            u32 size = kerning_offset_count;
            u32 i = 0;
            u32 index = 0;
            while (i < size) {
                index = i + size;
                index = index >> 1;
                
                if (kerning_array[index].character == character) {
                    return std::addressof(kerning_array[index]);
                }
                if (character < kerning_array[index].character) {
                    i = index + 1;
                    index = size;
                }
                size = index;
            }

            return nullptr;
        }
    };
    static_assert(sizeof(ResBffntKerningTable) == 0xc);
}
