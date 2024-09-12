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

    enum class SampleFormat {
        Invalid = 0,
        Pcm8 = 1,
        Pcm16 = 2,
        Pcm24 = 3,
        Pcm32 = 4,
        PcmFloat = 5,
        Adpcm = 6,
    };

    enum class BwavEncoding : u16 {
        Pcm16 = 0,
        Adpcm = 1,
        Opus = 2,
    };

    enum class BwavChannelPan : u16 {
        FrontLeft = 0,
        FrontRight = 1,
        Center = 2,
        LowFrequencyEffects = 3,
        LeftSurround = 4,
        RightSurround = 5,
        Invalid = 6,
    };

    struct AdpcmContext {
        u16 predictor_scale;
        s16 history_array[2];
    };
    static_assert(sizeof(AdpcmContext) == 0x6);

    struct ResBwavLoopPoint {
        u32 loop_start;
        AdpcmContext adpcm_context;
    };
    static_assert(sizeof(ResBwavLoopPoint) == 0xc);

    struct ResBwavChannelInfo {
        u16 encoding;
        u16 channel_pan;
        u32 sample_rate;
        u32 non_prefetch_sample_count;
        u32 sample_count;
        s16 adpcm_coefficients[16];
        u32 non_prefetch_samples_offset;
        u32 samples_offset;
        u16 loop_point_count;
        u16 base_loop_point;
        u32 loop_end;
        ResBwavLoopPoint loop_point_array[0];

        static constexpr u32 cInvalidLoopEnd = 0xffff'ffff;

        AdpcmContext GetAdpcmContext(u32 sample_position) {
            for (u32 i = 0; i < this->loop_point_count; ++i) {
                const ResBwavLoopPoint *loop_point = std::addressof(this->loop_point_array[i]);
                if (sample_position == loop_point->loop_start) { return loop_point->adpcm_context; }
                if (sample_position < loop_point->loop_start) { return (0 < i) ? this->loop_point_array[i - 1].adpcm_context : loop_point->adpcm_context; }
            }
            VP_ASSERT(this->loop_point_count != 0);
            return this->loop_point_array[0].adpcm_context;
        }
    };
    static_assert(sizeof(ResBwavChannelInfo) == 0x40);

    struct ResBwav {
        u32 magic;
        u16 endianess;
        u16 version;
        u32 sample_array_crc32;
        union {
            u16 flags;
            struct {
                u16 is_prefetch : 1;
                u16 reserve0 : 15;
            };
        };
        u16 channel_count;

        static constexpr u32 cMagic = util::TCharCode32("BWAV");
        static constexpr u32 cTargetVersion = 1;
        static constexpr u32 cSampleArrayAlignment = 0x40;

        constexpr bool IsValid() const {
            if (magic == cMagic && endianess == ByteOrder_Native && version == cTargetVersion) { return true; }
            return false;
        }

        ResBwavChannelInfo *GetChannelInfo(u32 channel_index) const {
            VP_ASSERT(channel_index < this->channel_count);
            uintptr_t address_iter = reinterpret_cast<uintptr_t>(this) + sizeof(ResBwav);
            for (u32 i = 0; i < channel_index; ++i) {
                ResBwavChannelInfo *channel_info = reinterpret_cast<ResBwavChannelInfo*>(address_iter);
                address_iter += sizeof(ResBwavChannelInfo) + channel_info->loop_point_count * sizeof(ResBwavLoopPoint);
            }
            return reinterpret_cast<ResBwavChannelInfo*>(address_iter);
        }

        BwavEncoding GetEncoding(u32 channel_index) {
            ResBwavChannelInfo *channel_info = this->GetChannelInfo(channel_index);
            return static_cast<BwavEncoding>(channel_info->encoding);
        }

        BwavChannelPan GetChannelPan(u32 channel_index) {
            ResBwavChannelInfo *channel_info = this->GetChannelInfo(channel_index);
            return static_cast<BwavChannelPan>(channel_info->channel_pan);
        }

        s16 *GetAdpcmCoefficients(u32 channel_index) {
            ResBwavChannelInfo *channel_info = this->GetChannelInfo(channel_index);
            return channel_info->adpcm_coefficients;
        }

        u32 GetSampleCount(u32 channel_index) {
            ResBwavChannelInfo *channel_info = this->GetChannelInfo(channel_index);
            return channel_info->non_prefetch_sample_count;
        }

        u32 GetSampleRate(u32 channel_index) {
            ResBwavChannelInfo *channel_info = this->GetChannelInfo(channel_index);
            return channel_info->sample_rate;
        }

        SampleFormat GetSampleFormat(u32 channel_index) {
            BwavEncoding encoding = this->GetEncoding(channel_index);
            return (encoding == BwavEncoding::Adpcm) ? SampleFormat::Adpcm : SampleFormat::Pcm16;
        }
    };
    static_assert(sizeof(ResBwav) == 0x10);
}
