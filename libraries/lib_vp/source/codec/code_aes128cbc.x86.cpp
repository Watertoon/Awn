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
#include <vp.hpp>

namespace vp::codec {

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
        context.Initialize(key, key_size, true);

        char iv_array[0x10]= {};
        if (init_vector != nullptr) {
            ::memcpy(iv_array, init_vector, 0x10);            
        }

        DecryptBlocksAes128CbcImpl(reinterpret_cast<vp::util::sse4::v2sll*>(output), reinterpret_cast<vp::util::sse4::v2sll*>(iv_array), reinterpret_cast<const vp::util::sse4::v2sll*>(input), input_size / sizeof(vp::util::sse4::v2sll), std::addressof(context));

        return;
    }

    void EncryptBlocksAes128CbcImpl(vp::util::sse4::v2sll *output, vp::util::sse4::v2sll *init_vector, const vp::util::sse4::v2sll *input, u32 block_count, Aes128Context *aes128_context) {

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
            round_iter = input_i ^ key ^ iv_iter;
            round_iter = Aesenc(round_iter,     round_key0);
            round_iter = Aesenc(round_iter,     round_key1);
            round_iter = Aesenc(round_iter,     round_key2);
            round_iter = Aesenc(round_iter,     round_key3);
            round_iter = Aesenc(round_iter,     round_key4);
            round_iter = Aesenc(round_iter,     round_key5);
            round_iter = Aesenc(round_iter,     round_key6);
            round_iter = Aesenc(round_iter,     round_key7);
            round_iter = Aesenc(round_iter,     round_key8);
            *output    = Aesenclast(round_iter, round_key9);

            /* Advance iters */
            output  = output + 1;
            input   = input  + 1;
        }
        *init_vector = *output;

        return;
    }

	void EncryptAes128Cbc(void *output, [[maybe_unused]] u32 output_size, void *key, u32 key_size, void *init_vector, [[maybe_unused]] u32 iv_size, const void *input, u32 input_size) {

        Aes128Context context = {};
        context.Initialize(key, key_size, false);

        char iv_array[0x10]= {};
        if (init_vector != nullptr) {
            ::memcpy(iv_array, init_vector, 0x10);            
        }

        EncryptBlocksAes128CbcImpl(reinterpret_cast<vp::util::sse4::v2sll*>(output), reinterpret_cast<vp::util::sse4::v2sll*>(iv_array), reinterpret_cast<const vp::util::sse4::v2sll*>(input), input_size / sizeof(vp::util::sse4::v2sll), std::addressof(context));

        return;
    }
}
