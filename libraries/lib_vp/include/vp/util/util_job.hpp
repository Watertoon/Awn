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
    
namespace vp::util {

    class Job {
        private:
            const char *m_job_name;
        public:
            constexpr ALWAYS_INLINE Job() : m_job_name(nullptr) {/*...*/}
            constexpr ALWAYS_INLINE Job(const char *name) : m_job_name(name) {/*...*/}
            constexpr ~Job() {/*...*/}

            virtual void Invoke() {/*...*/}

            constexpr ALWAYS_INLINE void SetName(const char *name) { m_job_name = name; }
    };
}
