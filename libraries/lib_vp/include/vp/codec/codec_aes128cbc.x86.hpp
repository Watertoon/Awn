#pragma once

namespace vp::codec {

    ALWAYS_INLINE vp::util::sse4::v2sll Aesimc(vp::util::sse4::v2sll round_key) {
        return __builtin_ia32_aesimc128(round_key);
    }

    ALWAYS_INLINE vp::util::sse4::v2sll Aesdec(vp::util::sse4::v2sll a, vp::util::sse4::v2sll round_key) {
        return __builtin_ia32_aesdec128(a, round_key);
    }

    ALWAYS_INLINE vp::util::sse4::v2sll Aesdeclast(vp::util::sse4::v2sll a, vp::util::sse4::v2sll round_key) {
        return __builtin_ia32_aesdeclast128(a, round_key);
    }
    //vp::util::sse4::v2sll Aeskeygenassist(vp::util::sse4::v2sll a, const u32 round_constant) {
    //    return __builtin_ia32_aeskeygenassist128(a, round_constant);
    //}

    struct Aes128Context {
        vp::util::sse4::v2sll key;
        vp::util::sse4::v2sll round_key0;
        vp::util::sse4::v2sll round_key1;
        vp::util::sse4::v2sll round_key2;
        vp::util::sse4::v2sll round_key3;
        vp::util::sse4::v2sll round_key4;
        vp::util::sse4::v2sll round_key5;
        vp::util::sse4::v2sll round_key6;
        vp::util::sse4::v2sll round_key7;
        vp::util::sse4::v2sll round_key8;
        vp::util::sse4::v2sll round_key9;

        static constexpr u8 cKeyGenArray[] = { 0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, 0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, 0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, 0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, 0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };
        static constexpr u8 cRoundConstantArray[] = { 0x6c, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91 };

        void Initialize(void *key, u32 key_size) {
            //VP_ASSERT(key_size <= sizeof(Aes128Context) && sizeof(vp::util::sse4::v2sll) <= key_size);

            /* Copy key data */
            ::memcpy(this, key, key_size);

            /* Generate round key data if necessary */
            const u32 round_key_count = key_size >> 2;
            if (key_size < sizeof(Aes128Context)) {
                //VP_ASSERT(round_key_count < 7);

                u32 key_gen_iter = reinterpret_cast<u32*>(this)[round_key_count - 1];
                u32 key_index    = 0;
                for (u32 i = round_key_count; i < 0x2c; ++i) {

                    /* Key gen adjust */
                    const u32 base_index = round_key_count + key_index;
                    const u32 round_constant_i = (round_key_count != 0) ? base_index / round_key_count : 0;
                    if (base_index == (round_constant_i * round_key_count)) {
                        const u32 gen0           = static_cast<u32>(cKeyGenArray[(key_gen_iter >> 0x8) & 0xff]);
                        const u32 gen1           = static_cast<u32>(cKeyGenArray[(key_gen_iter >> 0x10) & 0xff]);
                        const u32 gen2           = static_cast<u32>(cKeyGenArray[(key_gen_iter >> 0x18) & 0xff]);
                        const u32 gen3           = static_cast<u32>(cKeyGenArray[(key_gen_iter >> 0x0) & 0xff]);
                        const u32 round_constant = static_cast<u32>(cRoundConstantArray[round_constant_i]);
                        key_gen_iter = ((gen0) | (gen1 << 0x8) | (gen2 << 0x10) | (gen3 << 0x18)) ^ round_constant;
                    }

                    const u32 next_key_gen = reinterpret_cast<u32*>(this)[key_index] ^ key_gen_iter;
                    ++key_index;
                    reinterpret_cast<u32*>(this)[i] = next_key_gen;
                    key_gen_iter                   = next_key_gen;
                }
            }

            /* Inverse mix round keys */
            round_key0 = Aesimc(round_key0);
            round_key1 = Aesimc(round_key1);
            round_key2 = Aesimc(round_key2);
            round_key3 = Aesimc(round_key3);
            round_key4 = Aesimc(round_key4);
            round_key5 = Aesimc(round_key5);
            round_key6 = Aesimc(round_key6);
            round_key7 = Aesimc(round_key7);
            round_key8 = Aesimc(round_key8);

            return;
        }
    };

    void DecryptBlocksAes128CbcImpl(vp::util::sse4::v2sll *output, vp::util::sse4::v2sll *init_vector, const vp::util::sse4::v2sll *input, u32 block_count, Aes128Context *aes128_context) {

        const vp::util::sse4::v2sll key        = aes128_context->key;
        const vp::util::sse4::v2sll round_key0 = aes128_context->round_key0;
        const vp::util::sse4::v2sll round_key1 = aes128_context->round_key1;
        const vp::util::sse4::v2sll round_key2 = aes128_context->round_key2;
        const vp::util::sse4::v2sll round_key3 = aes128_context->round_key3;
        const vp::util::sse4::v2sll round_key4 = aes128_context->round_key4;
        const vp::util::sse4::v2sll round_key5 = aes128_context->round_key5;
        const vp::util::sse4::v2sll round_key6 = aes128_context->round_key6;
        const vp::util::sse4::v2sll round_key7 = aes128_context->round_key7;
        const vp::util::sse4::v2sll round_key8 = aes128_context->round_key8;
        const vp::util::sse4::v2sll round_key9 = aes128_context->round_key9;

        vp::util::sse4::v2sll iv_iter = *init_vector;

        for (u32 i = 0; i < block_count; ++i) {

            /* Perform ten rounds of aes decryption on input */
            const vp::util::sse4::v2sll input_i = *input;
            vp::util::sse4::v2sll       round_iter;
            round_iter = input_i ^ round_key9;
            round_iter = Aesdec(round_iter,     round_key8);
            round_iter = Aesdec(round_iter,     round_key7);
            round_iter = Aesdec(round_iter,     round_key6);
            round_iter = Aesdec(round_iter,     round_key5);
            round_iter = Aesdec(round_iter,     round_key4);
            round_iter = Aesdec(round_iter,     round_key3);
            round_iter = Aesdec(round_iter,     round_key2);
            round_iter = Aesdec(round_iter,     round_key1);
            round_iter = Aesdec(round_iter,     round_key0);
            *output    = iv_iter ^ Aesdeclast(round_iter, key);

            /* Advance iters */
            output  = output + 1;
            input   = input  + 1;
            iv_iter = input_i;
        }
        *init_vector = iv_iter;

        return;
    }

	void DecryptAes128Cbc(void *output, [[maybe_unused]] u32 output_size, void *key, u32 key_size, void *init_vector, [[maybe_unused]] u32 iv_size, const void *input, u32 input_size) {

        Aes128Context context = {};
        context.Initialize(key, key_size);

        char iv_array[0x10]= {};
        if (init_vector != nullptr) {
            ::memcpy(iv_array, init_vector, 0x10);            
        }

        DecryptBlocksAes128CbcImpl(reinterpret_cast<vp::util::sse4::v2sll*>(output), reinterpret_cast<vp::util::sse4::v2sll*>(iv_array), reinterpret_cast<const vp::util::sse4::v2sll*>(input), input_size / sizeof(vp::util::sse4::v2sll), std::addressof(context));

        return;
    }
}
