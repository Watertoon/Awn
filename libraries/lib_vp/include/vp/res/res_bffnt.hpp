#pragma once

namespace vp::res {

    struct ResBffnt : public ResUi2dHeader {
        static constexpr inline u32 Magic = util::TCharCode32("FFNT");
    };
    static_assert(sizeof(ResBffnt) == 0x14);

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
        u8  font_type;
        u8  height;
        u8  width;
        u8  ascent;
        u16 line_feed;
        u16 alternate_char_index;
        u8  default_left_width;
        u8  default_glyph_width;
        u8  default_character_width;
        u8  character_encoding;
        u32 texture_glyph_section_offset;
        u32 character_width_section_offset;
        u32 code_map_section_offset;

        static constexpr inline u32 Magic = util::TCharCode32("FINF");
    };
    static_assert(sizeof(ResBffntFontInfo) == 0x20);

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

        static constexpr inline u32 Magic = util::TCharCode32("TGLP");
    };
    static_assert(sizeof(ResBffntTextureGlyph) == 0x20);

    struct ResBffntCharacterWidthData {
        s8 left_width;
        u8 glyph_width;
        u8 char_width;
    };

    struct ResBffntCharacterWidth : public ResUi2dSection {
        u16                        first_entry_index;
        u16                        last_entry_index;
        u32                        next_character_width_offset;
        ResBffntCharacterWidthData width_data_array[];

        static constexpr inline u32 Magic = util::TCharCode32("CWDH");
    };
    static_assert(sizeof(ResBffntCharacterWidth) == 0x10);

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

        static constexpr inline u32 Magic = util::TCharCode32("KRNG");

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

    enum class BffntMapMethod {
        Direct = 0x0,
        Table  = 0x1,
        Scan   = 0x2,
    };

    struct ResBffntCodeMap : public ResUi2dSection {
        u32 code_range_begin;
        u32 code_range_end;
        u16 map_method;
        u16 reserve0;
        u32 next_code_map_offset;

        static constexpr inline u32 Magic = util::TCharCode32("CMAP");

        ResBffntCodeMap *GetNextCodeMap() const {
            return reinterpret_cast<ResBffntCodeMap*>(reinterpret_cast<uintptr_t>(this) + next_code_map_offset + sizeof(ResUi2dSection));
        }

        u16 *GetCodeMapArray() const {
            return reinterpret_cast<u16*>(reinterpret_cast<uintptr_t>(this) + sizeof(ResBffntCodeMap));
        }
    };
    static_assert(sizeof(ResBffntCodeMap) == 0x18);
}
