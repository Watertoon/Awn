#pragma once

namespace vp::res {

    struct ResRsizetableCrc32 {
        u32 path_crc32;
        u32 resource_size;
    };
    static_assert(sizeof(ResRsizetableCrc32) == 0x8);

	struct ResRsizetableOld {
		u32                magic;
        u32                resource_size_crc32_count;
        u32                resource_size_collision_count;
        ResRsizetableCrc32 resource_size_crc32_array[];

        static constexpr inline u32 cMagic         = util::TCharCode32("RSTB");

        ALWAYS_INLINE void *GetCollisionArray() {
            return (resource_size_collision_count != 0) ? reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + sizeof(ResRsizetableOld) + resource_size_crc32_count * sizeof(ResRsizetableCrc32)) : nullptr;
        }
	};
    static_assert(sizeof(ResRsizetableOld) == 0xc);

    #pragma pack(2)
    struct ResRsizetable {
		u32                magic0;
        u16                magic1;
        u32                version;
        u32                max_path_length;
        u32                resource_size_crc32_count;
        u32                resource_size_collision_count;
        ResRsizetableCrc32 resource_size_crc32_array[];

        static constexpr inline u32 cMagic0 = util::TCharCode32("REST");
        static constexpr inline u32 cMagic1 = util::TCharCode16("BL");
        static constexpr inline u32 cTargetVersion = 1;

        ALWAYS_INLINE void *GetCollisionArray() {
            return (resource_size_collision_count != 0) ? reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + sizeof(ResRsizetable) + resource_size_crc32_count * sizeof(ResRsizetableCrc32)) : nullptr;
        }
	};
    static_assert(sizeof(ResRsizetable) == 0x16);
    #pragma pack()

    class ResourceSizeTableExtractor {
        public:
            static constexpr inline u32 cInvalidSize      = 0xffff'ffff;
            static constexpr inline u32 cDefaultMaxLength = 0x80;
        private:
            ResRsizetableCrc32 *m_resource_size_crc32_array;
            void               *m_resource_size_collision_array;
            u32                 m_resource_size_crc32_count;
            u32                 m_resource_size_collision_count;
            u32                 m_max_path;
        public:
            constexpr ALWAYS_INLINE ResourceSizeTableExtractor() {/*...*/}

            bool Initialize(void *file, u32 file_size, u32 max_path_length = 0xffff'ffff) {

                /* Integrity checks */
                if (file == nullptr || file_size < sizeof(ResRsizetableCrc32)) { return false; }

                /* Find header type */
                const u32 magic = *reinterpret_cast<u32*>(file);
                if (magic == ResRsizetable::cMagic0 && *(reinterpret_cast<u16*>(file) + 2) == ResRsizetable::cMagic1){

                    /* Parse table */
                    ResRsizetable *table = reinterpret_cast<ResRsizetable*>(file);
                    m_resource_size_crc32_array      = table->resource_size_crc32_array;
                    m_resource_size_collision_array  = table->GetCollisionArray();
                    m_resource_size_crc32_count      = table->resource_size_crc32_count;
                    m_resource_size_collision_count  = table->resource_size_collision_count;
                    m_max_path                       = table->max_path_length;
                } else if (magic == ResRsizetableOld::cMagic) {

                    /* Parse old table */
                    ResRsizetableOld *table = reinterpret_cast<ResRsizetableOld*>(file);
                    m_resource_size_crc32_array     = table->resource_size_crc32_array;
                    m_resource_size_collision_array = table->GetCollisionArray();
                    m_resource_size_crc32_count     = table->resource_size_crc32_count;
                    m_resource_size_collision_count = table->resource_size_collision_count;
                    m_max_path                      = (max_path_length != 0xffff'ffff) ? max_path_length : cDefaultMaxLength;
                } else {

                    /* Parse headerless table*/
                    m_resource_size_crc32_array = reinterpret_cast<ResRsizetableCrc32*>(file);
                    m_resource_size_crc32_count = file_size / sizeof(ResRsizetableCrc32);
                    m_max_path                  = (max_path_length != 0xffff'ffff) ? max_path_length : cDefaultMaxLength;
                }

                return true;
            }

            constexpr void Finalize() {
                m_resource_size_crc32_array     = nullptr;
                m_resource_size_collision_array = nullptr;
                m_resource_size_crc32_count     = 0;
                m_resource_size_collision_count = 0;
                m_max_path                      = 0;
            }

            u32 TryGetResourceSize(const char *path) {

                /* Try to get size by crc32 */
                const u32 hash = vp::util::HashCrc32b(path);
                const u32 size = this->TryGetResourceSizeByCrc32(hash);

                /* Complete if the size was found */
                if (size != cInvalidSize) { return size; }

                /* Check collision table if necessary */
                return this->TryGetResourceSizeByPathCollision(path);
            }

            u32 TryGetResourceSizeByPathCollision(const char *path) {

                /* Integrity checks for collision table */
                if (m_resource_size_collision_array == nullptr || m_resource_size_collision_count == 0) { return cInvalidSize; }

                /* Get collision array */
                const size_t collision_size  = sizeof(u32) + m_max_path;
                uintptr_t    collision_array = reinterpret_cast<uintptr_t>(m_resource_size_collision_array);

                /* Binary search for path */
                u32 size  = m_resource_size_collision_count;
                u32 i     = 0;
                u32 index = 0;
                while (i < size) {
                    index = i + size;
                    index = index >> 1;
                    const char *collision_path = reinterpret_cast<const char*>(collision_array + collision_size * index);
                    const s32 result = ::strncmp(path, collision_path, m_max_path);

                    if (result == 0) { 
                        return *reinterpret_cast<u32*>(collision_array + collision_size * index + m_max_path); 
                    }
                    if (0 < result) {
                        i = index + 1;
                        index = size;
                    }
                    size = index;
                }

                return cInvalidSize;
            }
            u32 TryGetResourceSizeByCrc32(u32 hash_crc32) {

                /* Binary search for path hash */
                u32 entry_id  = m_resource_size_crc32_count / 2;
                u32 high_iter = m_resource_size_crc32_count;
                u32 low_iter  = 0;
                u32 i = 0;
                u32 current_hash = m_resource_size_crc32_array[entry_id].path_crc32;
                while (hash_crc32 != current_hash) {

                    u32 result = entry_id;
                    i = low_iter;
                    if (hash_crc32 < current_hash) {
                        i = high_iter;
                        high_iter = entry_id;
                        result = low_iter;
                    }

                    low_iter = result;
                    if (i == entry_id) { return cInvalidSize; }

                    i = high_iter + low_iter;

                    entry_id = i / 2;
                    current_hash = m_resource_size_crc32_array[entry_id].path_crc32;
                }

                return m_resource_size_crc32_array[entry_id].resource_size;
            }
    };
}
