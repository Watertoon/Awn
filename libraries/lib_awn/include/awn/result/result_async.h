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

namespace awn::async {
    
    DECLARE_RESULT_MODULE(13);
    DECLARE_RESULT(Incomplete,      1);
    DECLARE_RESULT(AlreadyQueued,   2);
    DECLARE_RESULT(InvalidPriority, 3);
    DECLARE_RESULT(Rescheduled,     4);
    DECLARE_RESULT(NoTask,          5);
    DECLARE_RESULT(DifferentQueue,  6);
    DECLARE_RESULT(IsNotPending,    7);
    DECLARE_RESULT(TaskExecuting,   8);
    DECLARE_RESULT(TaskExecuted,    9);
}
