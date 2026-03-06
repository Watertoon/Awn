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

    static constexpr u8 cCodeLengthTableUtf8[0x100] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 8,
    };

    static constexpr u32 cInvalidCodePointCount = 0xffff'ffff;

    template <typename T>
        requires (IsRawString8<T>)
    constexpr u32 GetCodePointCountOfUtf8String(const T *string, u32 size) {
        u32 cp_count = 0;
        if (size == 0) { return cp_count; }
        do {
            const u8 cp0 = static_cast<u8>(string[0]);
            const u8 cp_len = cCodeLengthTableUtf8[cp0];
            if (size < cp_len) { return cInvalidCodePointCount; }
            switch (cp_len) {
                case 0:
                default:
                    return cInvalidCodePointCount;
                case 1:
                    break;
                case 2:
                {
                    const u8 cp1 = static_cast<u8>(string[1]);
                    if ((cp1 & 0xc0) != 0x80) { return cInvalidCodePointCount; }
                }
                case 3:
                {
                    const u8 cp1 = static_cast<u8>(string[1]);
                    if (cp0 == 0xe0 && ((cp1 >> 5) & 1) == 0) { return cInvalidCodePointCount; }
                    if (cp0 == 0xed && ((cp1 >> 5) & 1) != 0) { return cInvalidCodePointCount; }
                    if ((cp1 & 0xc0) != 0x80) { return cInvalidCodePointCount; }
                    const u8 cp2 = static_cast<u8>(string[2]);
                    if ((cp2 & 0xc0) != 0x80) { return cInvalidCodePointCount; }
                    break;
                }
                case 4:
                {
                    const u8 cp1 = static_cast<u8>(string[1]);
                    if (cp0 == 0xf0 && (cp1 & 0x30) == 0) { return cInvalidCodePointCount; }
                    if (cp0 == 0xf4 && (cp1 & 0x30) != 0) { return cInvalidCodePointCount; }
                    if ((cp1 & 0xc0) != 0x80) { return cInvalidCodePointCount; }
                    const u8 cp2 = static_cast<u8>(string[2]);
                    if ((cp2 & 0xc0) != 0x80) { return cInvalidCodePointCount; }
                    const u8 cp3 = static_cast<u8>(string[3]);
                    if ((cp3 & 0xc0) != 0x80) { return cInvalidCodePointCount; }
                    break;
                }
            };
            string += cp_len;
            size -= cp_len;
            ++cp_count;
        } while (size != 0);
        return cp_count;
    }

    namespace impl {

        template <typename T16, typename T8>
            requires (IsRawString16<T16>) && (IsRawString8<T8>)
        constexpr u32 ConvertStringUtf16ToUtf8Impl(u32 *out_write_size, u32 *out_read_size, T8 *out_utf8, u32 max_out_length, const T16 *in_utf16, u32 in_length) {
            
            /* Nothing to convert if no input */
            if (in_length == 0) {
                *out_write_size = 0;
                *out_read_size = 0;
                return 0;
            }

            /* Setup stream state */
            u16 utf16_char    = *in_utf16;
            u32 in_left_iter  = in_length;
            u32 out_left_iter = max_out_length;

            /* Update output for successful exit */
            auto success_exit_guard = SCOPE_GUARD {
                *out_write_size = max_out_length - (out_left_iter - 1);
                *out_read_size  = in_length;
            };
            /* Update output for input error exit */
            auto input_error_guard = SCOPE_GUARD {
                *out_write_size = max_out_length - out_left_iter;
                *out_read_size  = in_length - in_left_iter;
            };

            /* Convert characters */
            do {
                
                if (utf16_char < 0x80) {
                    if (out_left_iter < 1) { success_exit_guard.Cancel(); return 1; }
                    *out_utf8 = static_cast<char>(utf16_char);
                    out_utf8 += 1;
                    out_left_iter -= 1;
                } else if (utf16_char < 0x800) {
                    if (out_left_iter < 2) { success_exit_guard.Cancel(); return 1; }
                    out_utf8[0] = (utf16_char >> 6) | 0xc0;
                    out_utf8[1] = 0x80 | (utf16_char & 0x3f);
                    out_utf8 += 2;
                    out_left_iter -= 2;
                } else if ((((utf16_char + 0x2000) >> 0xb) & 0x1f) < 0x1f) {
                    if (out_left_iter < 3) { success_exit_guard.Cancel(); return 1; }
                    out_utf8[0] = (utf16_char >> 0xc) | 0xe0;
                    out_utf8[1] = ((utf16_char >> 6) & 0x3f) | 0x80;
                    out_utf8[2] = 0x80 | (utf16_char & 0x3f);
                    out_utf8 += 3;
                    out_left_iter -= 3;
                } else {
                    if (in_left_iter == 1) {
                        if ((utf16_char >> 0xa) & 1) { 
                            success_exit_guard.Cancel();
                            return 2;
                        }
                        const u32 adjust = (utf16_char << 0xa) + 0x21'0000;
                        out_utf8[0] = (adjust >> 0x12) | 0xf0;
                        out_left_iter -= 1;
                        success_exit_guard.Cancel();
                        return 2;
                    }
                    in_utf16 += 1;
                    if ((utf16_char & 0xf800) == 0xd800) {
                        if ((utf16_char >> 0xa) & 1) { 
                            success_exit_guard.Cancel();
                            return 2;
                        }
                        utf16_char = *in_utf16 + (utf16_char << 0xa) + 0xfca02400;
                        if ((*in_utf16 & 0xfc00) != 0xdc00) {
                            const u32 adjust = (utf16_char << 0xa) + 0x21'0000;
                            out_utf8[0] = (adjust >> 0x12) | 0xf0;
                            out_left_iter -= 1;
                            success_exit_guard.Cancel();
                            return 2;
                        }
                    }
                    if (out_left_iter < 4) { success_exit_guard.Cancel(); return 1; }
                    out_utf8[0] = (utf16_char >> 0x12) | 0xf0;
                    out_utf8[1] = ((utf16_char >> 0xc) & 0x3f) | 0x80;
                    out_utf8[2] = ((utf16_char >> 6) & 0x3f) | 0x80;
                    out_utf8[3] = 0x80 | (utf16_char & 0x3f);
                    out_utf8 += 4;
                    out_left_iter -= 4;
                }

                in_left_iter -= 1;
                in_utf16 += 1;
                utf16_char = *in_utf16;
            } while (in_left_iter > 0);

            input_error_guard.Cancel();
            return 0;
        }

        template <typename T32, typename T8>
            requires (IsRawString32<T32>) && (IsRawString8<T8>)
        constexpr u32 ConvertStringUtf32ToUtf8Impl(u32 *out_write_size, u32 *out_read_size, T8 *out_utf8, u32 max_out_length, const T32 *in_utf32, u32 in_length) {

            /* Nothing to convert if no input */
            if (in_length == 0) {
                *out_write_size = 0;
                *out_read_size = 0;
                return 0;
            }

            /* Setup stream state */
            u32 utf32_char    = *in_utf32;
            u32 in_left_iter  = in_length;
            u32 out_left_iter = max_out_length;

            /* Update output for successful exit */
            auto success_exit_guard = SCOPE_GUARD {
                *out_write_size = max_out_length - (out_left_iter - 1);
                *out_read_size  = in_length;
            };
            /* Update output for input error exit */
            auto input_error_guard = SCOPE_GUARD {
                *out_write_size = max_out_length - out_left_iter;
                *out_read_size  = in_length - in_left_iter;
            };

            /* Convert characters */
            do {
                
                if (utf32_char < 0x80) {
                    if (out_left_iter < 1) { success_exit_guard.Cancel(); return 1; }
                    *out_utf8 = static_cast<char>(utf32_char);
                    out_utf8 += 1;
                    out_left_iter -= 1;
                } else if (utf32_char < 0x800) {
                    if (out_left_iter < 2) { success_exit_guard.Cancel(); return 1; }
                    out_utf8[0] = (utf32_char >> 6) | 0xc0;
                    out_utf8[1] = 0x80 | (utf32_char & 0x3f);
                    out_utf8 += 2;
                    out_left_iter -= 2;
                } else if ((utf32_char >> 0x10) == 0) {
                    if (out_left_iter < 3) { success_exit_guard.Cancel(); return 1; }
                    if ((utf32_char >> 0xb) == 0x1b) { success_exit_guard.Cancel(); return 2; }
                    out_utf8[0] = (utf32_char >> 0xc) | 0xe0;
                    out_utf8[1] = ((utf32_char >> 6) & 0x3f) | 0x80;
                    out_utf8[2] = 0x80 | (utf32_char & 0x3f);
                    out_utf8 += 3;
                    out_left_iter -= 3;
                } else {
                    if (0x10 < (utf32_char >> 0x10)) { success_exit_guard.Cancel(); return 2; }
                    if (out_left_iter < 4) { success_exit_guard.Cancel(); return 1; }
                    out_utf8[0] = (utf32_char >> 0x12) | 0xf0;
                    out_utf8[1] = ((utf32_char >> 0xc) & 0x3f) | 0x80;
                    out_utf8[2] = ((utf32_char >> 6) & 0x3f) | 0x80;
                    out_utf8[3] = 0x80 | (utf32_char & 0x3f);
                    out_utf8 += 4;
                    out_left_iter -= 4;
                }

                in_left_iter -= 1;
                in_utf32 += 1;
                utf32_char = *in_utf32;
            } while (in_left_iter > 0);

            input_error_guard.Cancel();
            return 0;
        }

        template <typename T16, typename T8>
            requires (IsRawString16<T16>) && (IsRawString8<T8>)
        constexpr u32 ConvertStringUtf8ToUtf16Impl(u32 *out_write_size, u32 *out_read_size, T16 *out_utf16, u32 max_out_length, const T8 *in_utf8, u32 in_length) {

            /* Nothing to convert if no input */
            if (in_length == 0) {
                *out_write_size = 0;
                *out_read_size = 0;
                return 0;
            }

            /* Setup stream state */
            u8  utf8_char0 = *in_utf8;
            u32 code_length = cCodeLengthTableUtf8[utf8_char0];
            u32 in_left_iter = in_length;
            u32 out_left_iter = max_out_length;

            /* Update output for successful exit */
            auto success_exit_guard = SCOPE_GUARD {
                *out_write_size = max_out_length - (out_left_iter - 1);
                *out_read_size  = in_length;
            };
            /* Update output for input error exit */
            auto input_error_guard = SCOPE_GUARD {
                *out_write_size = max_out_length - out_left_iter;
                *out_read_size  = in_length - in_left_iter;
            };

            /* Check complete for out of input error */
            while (code_length <= in_left_iter) {

                /* Check complete for out of output error */
                if (out_left_iter == 0) {
                    success_exit_guard.Cancel();
                    return 1;
                }

                /* Check complete for invalid code length input error (must be 1-4) */
                if (3 < code_length - 1) { success_exit_guard.Cancel(); return 2; }

                /* Convert character */
                switch (code_length) {
                    case 1:
                    {
                        /* Set utf16 char */
                        *out_utf16 = utf8_char0;

                        /* Advance in iter */
                        in_left_iter -= 1;

                        /* Check for completion */
                        if (in_left_iter == 0) { input_error_guard.Cancel(); return 0; }

                        /* Advance in string */
                        in_utf8 += 1;

                        break;
                    }
                    case 2:
                    {
                        /* Read next char */
                        const u8 utf8_char1 = in_utf8[1];

                        /* Check for invalid character input error */
                        if ((utf8_char0 & 0x1e) == 0 || ((utf8_char1 & 0xc0) != 0x80)) { success_exit_guard.Cancel(); return 2; }

                        /* Convert utf8 to utf16 character */
                        *out_utf16 = ((utf8_char0 & 0x1f) << 6) | (utf8_char1 & 0x3f);

                        /* Advance in iter */
                        in_left_iter -= 2;

                        /* Check for completion */
                        if (in_left_iter == 0) { input_error_guard.Cancel(); return 0; }

                        /* Advance in string */
                        in_utf8 += 2;

                        break;
                    }
                    case 3:
                    {
                        /* Read next char */
                        const u8 utf8_char1 = in_utf8[1];
                        const u8 utf8_char2 = in_utf8[2];

                        /* Check for invalid character input error */
                        if (((utf8_char1 & 0xc0) != 0x80) || ((utf8_char2 & 0xc0) != 0x80)) { success_exit_guard.Cancel(); return 2; }

                        /* Convert to utf16 and check input is valid */
                        const u32 convert_high = ((utf8_char0 & 0xf) << 0xc) | ((utf8_char1 & 0x3f) << 6);
                        const u32 check_value  = convert_high & 0xf800;
                        if (check_value == 0 || check_value == 0xd800) { success_exit_guard.Cancel(); return 2; }
                        *out_utf16 = convert_high | (utf8_char2 & 0x3f);

                        /* Advance in iter */
                        in_left_iter -= 3;

                        /* Check for completion */
                        if (in_left_iter == 0) { input_error_guard.Cancel(); return 0; }

                        /* Advance in string */
                        in_utf8 += 3;

                        break;
                    }
                    case 4:
                    {
                        /* Read next char */
                        const u8 utf8_char1 = in_utf8[1];
                        const u8 utf8_char2 = in_utf8[2];
                        const u8 utf8_char3 = in_utf8[3];

                        /* Check for invalid character input error */
                        if (((utf8_char1 & 0xc0) != 0x80) || ((utf8_char2 & 0xc0) != 0x80) || ((utf8_char3 & 0xc0) != 0x80)) { success_exit_guard.Cancel(); return 2; }


                        /* Convert to utf32 and check input is valid */
                        const u32 check_value  = ((utf8_char0 & 7) << 0x12) | ((utf8_char1 & 0x3f) << 0xc) | ((utf8_char2 & 0x3f) << 6) | (utf8_char3 & 0x3f);
                        if (((check_value - 0x10000) >> 0x14) != 0) { success_exit_guard.Cancel(); return 2; }
                        if (out_left_iter == 1) { success_exit_guard.Cancel(); return 1; }
                        out_utf16[0] = ((check_value + 0x3ff'0000) >> 0xa) + 0xd800;
                        out_utf16[1] = (check_value & 0x3ff) | 0xdc00;
                        out_utf16 += 1;

                        /* Advance in iter */
                        in_left_iter -= 4;

                        /* Check for completion */
                        if (in_left_iter == 0) { input_error_guard.Cancel(); return 0; }

                        /* Advance in string */
                        in_utf8 += 4;

                        break;
                    }
                    default:
                        break;
                };

                /* Read next char */
                utf8_char0 = *in_utf8;

                /* Advance output iters */
                out_utf16     += 1;
                out_left_iter -= 1;

                /* Get next code length */
                code_length = cCodeLengthTableUtf8[utf8_char0];
            }

            /* Out of input error */
            success_exit_guard.Cancel();
            return 2;
        }

        template <typename T32, typename T8>
            requires (IsRawString32<T32>) && (IsRawString8<T8>)
        constexpr u32 ConvertStringUtf8ToUtf32Impl(u32 *out_write_size, u32 *out_read_size, T32 *out_utf32, u32 max_out_length, const T8 *in_utf8, u32 in_length) {

            /* Nothing to convert if no input */
            if (in_length == 0) {
                *out_write_size = 0;
                *out_read_size = 0;
                return 0;
            }

            /* Setup stream state */
            u8  utf8_char0     = *in_utf8;
            u32 code_length   = cCodeLengthTableUtf8[utf8_char0];
            u32 in_left_iter  = in_length;
            u32 out_left_iter = max_out_length;

            /* Update output for successful exit */
            auto success_exit_guard = SCOPE_GUARD {
                *out_write_size = max_out_length - (out_left_iter - 1);
                *out_read_size  = in_length;
            };
            /* Update output for input error exit */
            auto input_error_guard = SCOPE_GUARD {
                *out_write_size = max_out_length - out_left_iter;
                *out_read_size  = in_length - in_left_iter;
            };

            /* Check complete for out of input error */
            while(code_length <= in_left_iter) {

                /* Check complete for out of output error */
                if (out_left_iter == 0) {
                    success_exit_guard.Cancel();
                    return 1;
                }

                /* Check complete for invalid code length input error (must be 1-4) */
                if (3 < code_length - 1) { success_exit_guard.Cancel(); return 2; }

                /* Convert character */
                switch (code_length) {
                    case 1:
                    {
                        /* Set utf32 char */
                        *out_utf32 = utf8_char0;

                        /* Advance in iter */
                        in_left_iter -= 1;

                        /* Check for completion */
                        if (in_left_iter == 0) { input_error_guard.Cancel(); return 0; }

                        /* Advance in string */
                        in_utf8 += 1;

                        break;
                    }
                    case 2:
                    {
                        /* Read next char */
                        const u8 utf8_char1 = in_utf8[1];

                        /* Check for invalid character input error */
                        if ((utf8_char0 & 0x1e) == 0 || cCodeLengthTableUtf8[utf8_char1] != 0) { success_exit_guard.Cancel(); return 2; }

                        /* Convert utf8 to utf32 character */
                        *out_utf32 = ((utf8_char0 & 0x1f) << 6) | (utf8_char1 & 0x3f);

                        /* Advance in iter */
                        in_left_iter -= 2;

                        /* Check for completion */
                        if (in_left_iter == 0) { input_error_guard.Cancel(); return 0; }

                        /* Advance in string */
                        in_utf8 += 2;

                        break;
                    }
                    case 3:
                    {
                        /* Read next char */
                        const u8 utf8_char1 = in_utf8[1];
                        const u8 utf8_char2 = in_utf8[2];

                        /* Check for invalid character input error */
                        if (cCodeLengthTableUtf8[utf8_char1] != 0 || cCodeLengthTableUtf8[utf8_char2] != 0) { success_exit_guard.Cancel(); return 2; }

                        /* Convert to utf32 and check input is valid */
                        const u32 convert_high = ((utf8_char0 & 0xf) << 0xc) | ((utf8_char1 & 0x3f) << 6);
                        const u32 check_value  = convert_high & 0xf800;
                        if (check_value == 0 || check_value == 0xd800) { success_exit_guard.Cancel(); return 2; }
                        *out_utf32 = convert_high | (utf8_char2 & 0x3f);

                        /* Advance in iter */
                        in_left_iter -= 3;

                        /* Check for completion */
                        if (in_left_iter == 0) { input_error_guard.Cancel(); return 0; }

                        /* Advance in string */
                        in_utf8 += 3;

                        break;
                    }
                    case 4:
                    {
                        /* Read next char */
                        const u8 utf8_char1 = in_utf8[1];
                        const u8 utf8_char2 = in_utf8[2];
                        const u8 utf8_char3 = in_utf8[3];

                        /* Check for invalid character input error */
                        if (cCodeLengthTableUtf8[utf8_char1] != 0 || cCodeLengthTableUtf8[utf8_char2] != 0) { success_exit_guard.Cancel(); return 2; }


                        /* Convert to utf32 and check input is valid */
                        const u32 check_value  = ((utf8_char0 & 7) << 0x12) | ((utf8_char1 & 0x3f) << 0xc) | ((utf8_char2 & 0x3f) << 6) | (utf8_char3 & 0x3f);
                        if (((check_value - 0x10000) >> 0x14) != 0) { success_exit_guard.Cancel(); return 2; }
                        *out_utf32 = check_value;

                        /* Advance in iter */
                        in_left_iter -= 4;

                        /* Check for completion */
                        if (in_left_iter == 0) { input_error_guard.Cancel(); return 0; }

                        /* Advance in string */
                        in_utf8 += 4;

                        break;
                    }
                    default:
                        break;
                };

                /* Read next char */
                utf8_char0 = *in_utf8;

                /* Advance output iters */
                out_utf32     += 1;
                out_left_iter -= 1;

                /* Get next code length */
                code_length = cCodeLengthTableUtf8[utf8_char0];
            }

            success_exit_guard.Cancel();
            return 2;
        }
    }

    template <typename T8>
        requires (IsRawString8<T8>)
    constexpr bool GetNextCharacterUtf8(T8 *out_utf8, const T8 **in_out_utf8_iter) {

        /* Load first character */
        const char *string     = *in_out_utf8_iter;
        const u8    utf8_char0 = string[0];

        /* Ensure valid code range */
        if (3 < cCodeLengthTableUtf8[utf8_char0] - 1) { return false; }
        
        /* Read out based on size */
        switch (cCodeLengthTableUtf8[utf8_char0]) {
            case 1:
            {
                /* Write out */
                out_utf8[0] = utf8_char0;

                /* Iterate */
                *in_out_utf8_iter = *in_out_utf8_iter + 1;

                break;
            }
            case 2:
            {
                /* Character check */
                if ((cCodeLengthTableUtf8[utf8_char0] & 0x1e) == 0) { return false; }

                /* Length check */
                const u8 utf8_char1 = string[1];
                if (cCodeLengthTableUtf8[utf8_char1] != 0) { return false; }

                /* Write out */
                out_utf8[0] = utf8_char0;
                out_utf8[1] = utf8_char1;

                /* Iterate */
                *in_out_utf8_iter = *in_out_utf8_iter + 2;

                break;
            }
            case 3:
            {
                /* Length check */
                const u8 utf8_char1 = string[1];
                if (cCodeLengthTableUtf8[utf8_char1] != 0) { return false; }
                const u8 utf8_char2 = string[2];
                if (cCodeLengthTableUtf8[utf8_char2] != 0) { return false; }

                /* Character check */
                const u32 check = ((utf8_char1 & 0x20) << 6) | ((utf8_char0 & 0xf) << 0xc);
                if (check == 0 || check == 0xd800) { return false; }

                /* Write out */
                out_utf8[0] = utf8_char0;
                out_utf8[1] = utf8_char1;
                out_utf8[2] = utf8_char2;

                /* Iterate */
                *in_out_utf8_iter = *in_out_utf8_iter + 3;
                
                break;
            }
            case 4:
            {
                /* Length check */
                const u8 utf8_char1 = string[1];
                if (cCodeLengthTableUtf8[utf8_char1] != 0) { return false; }
                const u8 utf8_char2 = string[2];
                if (cCodeLengthTableUtf8[utf8_char2] != 0) { return false; }
                const u8 utf8_char3 = string[3];
                if (cCodeLengthTableUtf8[utf8_char3] != 0) { return false; }

                /* Character check */
                const u32 check = ((((utf8_char1 & 0x3f) << 6) | ((utf8_char0 & 0x7) << 0x12)) - 0x10000) >> 0x14;
                if (check != 0) { return false; }

                /* Write out */
                out_utf8[0] = utf8_char0;
                out_utf8[1] = utf8_char1;
                out_utf8[2] = utf8_char2;
                out_utf8[2] = utf8_char2;

                /* Iterate */
                *in_out_utf8_iter = *in_out_utf8_iter + 4;
                
                break;
            }
        }

        return true;
    }

    template <typename T8>
        requires (IsRawString8<T8>)
    constexpr bool GetNextCharacterUtf8(u32 *out_utf8, const T8 **in_out_utf8_iter) {
        char char_arr[4] = {};
        const bool result = GetNextCharacterUtf8(char_arr, in_out_utf8_iter);
        *out_utf8 = std::bit_cast<u32>(char_arr);
        return result;
    }

    //constexpr u32 ConvertCharacterUtf8ToUtf16(char16_t *out_utf16, const char *in_utf8);

    template <typename T32, typename T8>
        requires (IsRawString32<T32>) && (IsRawString8<T8>)
    constexpr u32 ConvertCharacterUtf8ToUtf32(T32 *out_utf32, const T8 *in_utf8) {

        /* Read first character */
        const u8 utf8_char0 = in_utf8[0];
        u32      utf32_char = utf8_char0;
        u32      length     = 1;

        /* Ensure valid code range */
        if (3 < cCodeLengthTableUtf8[utf8_char0] - 1) { return 0; }

        /* Convert based on size */
        switch (cCodeLengthTableUtf8[utf8_char0]) {
            default:
            case 1:
                break;
            case 2:
            {
                /* Char0 check */
                if ((utf8_char0 & 0x1e) == 0) { return 0; } 

                /* Length check */
                const u8 utf8_char1 = in_utf8[1];
                if (cCodeLengthTableUtf8[utf8_char1] != 0) { return 0; }

                /* Combine */
                utf32_char = (utf8_char1 & 0x3f) | ((utf8_char0 & 0x1f) << 0x6);
                length = 2;

                break;
            }
            case 3:
            {
                /* Length check */
                const u8 utf8_char1 = in_utf8[1];
                if (cCodeLengthTableUtf8[utf8_char1] != 0) { return 0; }

                const u8 utf8_char2 = in_utf8[2];
                if (cCodeLengthTableUtf8[utf8_char2] != 0) { return 0; }

                /* Offset char data */
                const u32 offset_char0 = (utf8_char0 & 0xf) << 0xc;
                const u32 offset_char1 = (utf8_char1 & 0x3f) << 0x6;

                /* Validate offset data */
                const u32 combine_check = offset_char0 | (offset_char1 & 0xf800);
                if (combine_check == 0 || combine_check == 0xd800) { return 0; }

                /* Combine */
                utf32_char = (utf8_char2 & 0x3f) | offset_char1 | offset_char0;
                length = 3;

                break;
            }
            case 4:
            {
                /* Length check */
                const u8 utf8_char1 = in_utf8[1];
                if (cCodeLengthTableUtf8[utf8_char1] != 0) { return 0; }

                const u8 utf8_char2 = in_utf8[2];
                if (cCodeLengthTableUtf8[utf8_char2] != 0) { return 0; }

                const u8 utf8_char3 = in_utf8[3];
                if (cCodeLengthTableUtf8[utf8_char3] != 0) { return 0; }

                /* Offset char data */
                const u32 offset_char0 = (utf8_char0 & 0xf) << 0x12;
                const u32 offset_char1 = (utf8_char1 & 0x3f) << 0xc;
                const u32 offset_char2 = (utf8_char2 & 0x3f) << 0x6;

                /* Combine */
                utf32_char = (utf8_char3 & 0x3f) | offset_char2 | offset_char1 | offset_char0;
                length = 4;

                /* Validate offset data */
                if (((utf32_char - 0x1'0000) >> 0x14) != 0) { return 0; }

                break;
            }
        }

        /* Set utf32 character */
        *out_utf32 = utf32_char;

        return length;
    }

    //constexpr u32 ConvertCharacterUtf16ToUtf8(char *out_utf8, char16_t *in_utf16);

    template <typename T32, typename T8>
        requires (IsChar32<T32>) && (IsRawString8<T8>)
    constexpr u32 ConvertCharacterUtf32ToUtf8(T8 *out_utf8, T32 in_utf32) {
        
        /* Clear utf8 */
        out_utf8[0] = 0;
        out_utf8[1] = 0;
        out_utf8[2] = 0;
        out_utf8[3] = 0;

        /* Get first char */
        const u8 utf8_char0 = static_cast<u8>(in_utf32);
        
        /* 1-byte */
        if (in_utf32 < 0x80) {
            out_utf8[0] = utf8_char0;
            return 0;
        }

        /* 2-byte */
        if (in_utf32 < 0x800) {
            out_utf8[0] = (in_utf32 >> 6) | 0xc0;
            out_utf8[1] = 0x80 | (utf8_char0 & 0x3f);
            return 0;
        }

        /* 3-byte */
        if ((in_utf32 >> 0x10) == 0) {
            if ((in_utf32 >> 0xb) == 0x1b) { return 2; }
            out_utf8[0] = (in_utf32 >> 0xc) | 0xe0;
            out_utf8[1] = ((in_utf32 >> 6) & 0x3f) | 0x80;
            out_utf8[2] = 0x80 | (in_utf32 & 0x3f);
            return 0;
        }

        /* 4-byte */
        if ((in_utf32 >> 0x10) < 0x11) {
            out_utf8[0] = (in_utf32 >> 0x12) | 0xf0;
            out_utf8[1] = ((in_utf32 >> 0xc) & 0x3f) | 0x80;
            out_utf8[2] = ((in_utf32 >> 6) & 0x3f) | 0x80;
            out_utf8[3] = 0x80 | (in_utf32 & 0x3f);
            return 0;
        }

        return 2;
    }

    template <typename T16, typename T8>
        requires (IsRawString16<T16>) && (IsRawString8<T8>)
    constexpr u32 ConvertStringUtf8ToUtf16(u32 *out_size, T16 *out_utf16, u32 max_out_length, const T8 *in_utf8, u32 in_length) {
        u32 size_read  = 0;
        return impl::ConvertStringUtf8ToUtf16Impl(out_size, std::addressof(size_read), out_utf16, max_out_length, in_utf8, in_length);
    }

    template <typename T32, typename T8>
        requires (IsRawString32<T32>) && (IsRawString8<T8>)
    constexpr u32 ConvertStringUtf8ToUtf32(u32 *out_size, T32 *out_utf32, u32 max_out_length, const T8 *in_utf8, u32 in_length) {
        u32 size_read  = 0;
        return impl::ConvertStringUtf8ToUtf32Impl(out_size, std::addressof(size_read), out_utf32, max_out_length, in_utf8, in_length);
    }

    template <typename T16, typename T8>
        requires (IsRawString16<T16>) && (IsRawString8<T8>)
    constexpr u32 ConvertStringUtf16ToUtf8(u32 *out_size, T8 *out_utf8, u32 max_out_length, const T16 *in_utf16, u32 in_length) {
        u32 size_read  = 0;
        return impl::ConvertStringUtf16ToUtf8Impl(out_size, std::addressof(size_read), out_utf8, max_out_length, in_utf16, in_length);
    }

    template <typename T32, typename T8>
        requires (IsRawString32<T32>) && (IsRawString8<T8>)
    constexpr u32 ConvertStringUtf32ToUtf8(u32 *out_size, T8 *out_utf8, u32 max_out_length, const T32 *in_utf32, u32 in_length) {
        u32 size_read  = 0;
        return impl::ConvertStringUtf32ToUtf8Impl(out_size, std::addressof(size_read), out_utf8, max_out_length, in_utf32, in_length);
    }

    template<typename O, typename I>
    concept IsCharEncodingConvertible = 
       (IsChar<O> && IsChar<I> && sizeof(O) == sizeof(char8_t) && sizeof(I) == sizeof(char16_t)) 
    || (IsChar<O> && IsChar<I> && sizeof(O) == sizeof(char8_t) && sizeof(I) == sizeof(char32_t))
    || (IsChar<O> && IsChar<I> && sizeof(O) == sizeof(char16_t) && sizeof(I) == sizeof(char8_t))
    || (IsChar<O> && IsChar<I> && sizeof(O) == sizeof(char32_t) && sizeof(I) == sizeof(char8_t));

    template <typename O, typename I>
        requires (IsCharEncodingConvertible<O,I>)
    constexpr u32 ConvertStringEncoding(u32 *out_size, O *out_type, u32 max_out_length, const I *in_type, u32 in_length) {
        u32 size_read  = 0;
        if constexpr (sizeof(O) == sizeof(char8_t) && sizeof(I) == sizeof(char16_t)) {
            return impl::ConvertStringUtf16ToUtf8Impl(out_size, std::addressof(size_read), out_type, max_out_length, in_type, in_length);
        } else if constexpr (sizeof(O) == sizeof(char8_t) && sizeof(I) == sizeof(char32_t)) {
            return impl::ConvertStringUtf32ToUtf8Impl(out_size, std::addressof(size_read), out_type, max_out_length, in_type, in_length);
        } else if constexpr (sizeof(O) == sizeof(char16_t) && sizeof(I) == sizeof(char8_t)) {
            return impl::ConvertStringUtf8ToUtf16Impl(out_size, std::addressof(size_read), out_type, max_out_length, in_type, in_length);
        } else if constexpr (sizeof(O) == sizeof(char32_t) && sizeof(I) == sizeof(char8_t)) { 
            return impl::ConvertStringUtf8ToUtf32Impl(out_size, std::addressof(size_read), out_type, max_out_length, in_type, in_length);
        } else {
            _consteval_fail();
        }
    }
}
