#pragma once

namespace vp::resbui {

    DECLARE_RESULT_MODULE(5);
    DECLARE_RESULT(SectionExhaustion, 1);
    DECLARE_RESULT(EntryExhaustion,   2);
    DECLARE_RESULT(NullArgument,      3);
    DECLARE_RESULT(InvalidPath,       4);
    DECLARE_RESULT(DuplicatePath,     5);
    DECLARE_RESULT(AlreadyLinked,     6);
}
