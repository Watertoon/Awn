#pragma once

namespace vp::res {

    DECLARE_RESULT_MODULE(5);
    DECLARE_RESULT(NoMemory, 1);
    DECLARE_RESULT(EncodingNG, 2);
    DECLARE_RESULT(ValueNG, 2);
    DECLARE_RESULT(StreamNG, 3);
    DECLARE_RESULT(ParserNG, 4);
    DECLARE_RESULT(DataTypeNG, 5);
    DECLARE_RESULT(KeyCollision, 6);
}
