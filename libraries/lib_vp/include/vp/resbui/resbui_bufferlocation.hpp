#pragma once

namespace vp::resbui {

    struct BufferLocation {
        size_t offset;
        size_t size;
        s32    alignment;
        u32    relocation_count;
    };
}
