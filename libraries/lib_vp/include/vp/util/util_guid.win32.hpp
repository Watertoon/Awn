#pragma once

namespace vp::util {

    struct Guid {
        union {
            u8 guid[0x10];
            struct {
                u32 guid0;
                u16 guid1;
                u16 guid2;
                u8  guid3[2];
                u8  guid4[6];
            };
        };
    };
    static_assert(sizeof(Guid) == sizeof(GUID));

    ALWAYS_INLINE void GenerateGuid(Guid *guid) {
        const HRESULT result = ::CoCreateGuid(reinterpret_cast<GUID*>(guid));
        VP_ASSERT(result == S_OK);
    }
}
