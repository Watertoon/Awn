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
#include <awn/async.h>
#include <awn/res.h>
#include <awn/gfx.h>
#include <awn/hid.h>
#include <awn/frm.h>
