#include <vp.hpp>

namespace vp::codec {

	void GenerateNisasystKey(NisasystKey *out_key, vp::res::NintendoWareRandom *random) {

		/* Generate key */
        for (u32 i = 0; i < 0x10; ++i) {
            const char byte_hex[5] = { '0', 'x', cNisasystKeyGenString[(random->GetU32() >> 0x18) & 0xff], cNisasystKeyGenString[(random->GetU32() >> 0x18) & 0xff], '\0' };
            out_key->key_array[i]  = std::strtol(byte_hex, nullptr, 0x10);
        }

        return;
	}

    bool IsNisasystEncrypted(void *file, u32 file_size) {
        if (file_size < sizeof(u64)) { return false; }

        const u64 magic = *reinterpret_cast<u64*>(reinterpret_cast<uintptr_t>(file) + (file_size - sizeof(u64)));
        return magic == cNisasystMagic;
    }

	void DecryptNisasyst(void *in_out_file, u32 file_size, const char *file_path) {

		/* Initialize random to generate path based key */
		const u32 hash = vp::util::HashCrc32b(file_path);
		vp::res::NintendoWareRandom random(hash);

        /* Generate keys */
		NisasystKey key;
		NisasystKey init_vector;
        GenerateNisasystKey(std::addressof(key), std::addressof(random));
        GenerateNisasystKey(std::addressof(init_vector), std::addressof(random));

        /* Perform aes decryption */
        vp::codec::DecryptAes128Cbc(in_out_file, file_size, key.key_array, sizeof(NisasystKey), init_vector.key_array, sizeof(NisasystKey), in_out_file, file_size);

        return;
	}
}
