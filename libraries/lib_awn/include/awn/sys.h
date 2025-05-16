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

#include <awn/sys/sys_systemapi.h>

#ifdef VP_TARGET_PLATFORM_win32
    #include <awn/sys/sys_internalsynchronization.win32.h>
    #include <awn/sys/sys_mutex.win32.h>
    #include <awn/sys/sys_event.win32.hpp>
    #include <awn/sys/sys_readerwriterlock.win32.h>
    #include <awn/sys/sys_messagequeue.win32.h>
    #include <awn/sys/sys_systemevent.win32.hpp>
#elif VP_TARGET_PLATFORM_nx
    #include <awn/sys/sys_event.nx.hpp>
    #include <awn/sys/sys_messagequeue.nx.hpp>
#endif

#include <awn/sys/sys_threadbase.h>
#include <awn/sys/sys_threadapi.h>

#ifdef VP_TARGET_PLATFORM_win32
    #include <awn/sys/sys_thread.win32.hpp>
    #include <awn/sys/sys_mainthread.win32.hpp>
    #include <awn/sys/sys_threadmanager.win32.h>
#elif VP_TARGET_PLATFORM_nx
    #include <awn/sys/sys_thread.nx.hpp>
    #include <awn/sys/sys_mainthread.nx.hpp>
    #include <awn/sys/sys_threadmanager.nx.hpp>
#endif

#include <awn/sys/sys_functionthread.hpp>
