#pragma once

#include <vp.hpp>

namespace awn {
    using Result                   = vp::Result;
    using TimeSpan                 = vp::TimeSpan;
    using TickSpan                 = vp::TickSpan;
    constexpr Result ResultSuccess = vp::ResultSuccess;
}

#include <awn/result.h>
#include <awn/ukern.h>
#include <awn/mem.h>
#include <awn/sys.h>
#include <awn/res.h>
#include <awn/gfx.h>
#include <awn/hid.h>
#include <awn/frm.h>
