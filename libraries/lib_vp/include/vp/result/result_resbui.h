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

namespace vp::resbui {

    DECLARE_RESULT_MODULE(5);
    DECLARE_RESULT(SectionExhaustion, 1);
    DECLARE_RESULT(EntryExhaustion,   2);
    DECLARE_RESULT(NullArgument,      3);
    DECLARE_RESULT(InvalidPath,       4);
    DECLARE_RESULT(DuplicatePath,     5);
    DECLARE_RESULT(AlreadyLinked,     6);
}
