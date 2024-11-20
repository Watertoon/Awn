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

namespace awn::sys {

    class ReaderWriterLock {
        public:
            static constexpr u32 cTagBitU32 = 0xffff'ffff;
            static constexpr u32 cReaderCountMaskU32 = 0x0000'03ff;
            static constexpr u32 cReaderWaitCountMaskU32 = 0x000f'fc00;
            static constexpr u32 cWriterWaitCountMaskU32 = 0x3ff0'0000;
            static constexpr u32 cWriterLockBitU32 = 0x4000'0000;

            static constexpr u64 cTagBitU64 = 0x0000'0000'ffff'ffff;
            static constexpr u64 cReaderCountMaskU64 = 0x0000'03ff'0000'0000;
            static constexpr u64 cReaderWaitCountMaskU64 = 0x000f'fc00'0000'0000;
            static constexpr u64 cWriterWaitCountMaskU64 = 0x3ff0'0000'0000'0000;
            static constexpr u64 cWriterLockBitU64 = 0x4000'0000'0000'0000;
        private:
            union {
                impl::InternalCriticalSection m_cs;
                u64 m_state;
                struct {
                    u64 m_tag : 32;
                    u64 m_read_count : 10;
                    u64 m_read_waiter : 10;
                    u64 m_write_waiter : 10;
                    u64 m_is_writer : 1;
                    u64 m_reserve0 : 1;
                };
            };
            u16 m_writer_count;
            u16 m_writer_read_count;
            u32 m_writer_thread_id;
            impl::InternalConditionVariable m_read_cv;
            impl::InternalConditionVariable m_write_cv;
        private:
            void AcquireReadLockAsWriter();
            void ReleaseReadLockAsWriter();
            void ReleaseWriteLockImpl();
        public:
            constexpr ReaderWriterLock() : m_state(), m_writer_count(), m_writer_read_count(), m_writer_thread_id(), m_read_cv(), m_write_cv()  {/*...*/}
            constexpr ~ReaderWriterLock() {/*...*/}

            void AcquireWriteLock();
            bool TryAcquireWriteLock();
            void ReleaseWriteLock();

            void AcquireReadLock();
            bool TryAcquireReadLock();
            void ReleaseReadLock();

            bool IsWriteLockedByCurrentThread() const;
            constexpr bool IsReadLocked() const {
                const u64 counts = m_state;
                return 0 < ((counts >> 0x20) & cReaderCountMaskU32) || 0 < m_writer_read_count;
            }
    };

    class ScopedWriteLock {
        private:
            ReaderWriterLock *m_lock;
        public:
            ALWAYS_INLINE ScopedWriteLock(ReaderWriterLock *reader_writer_lock) : m_lock(reader_writer_lock) {
                reader_writer_lock->AcquireWriteLock();
            }
            ALWAYS_INLINE ~ScopedWriteLock() {
                m_lock->ReleaseWriteLock();
            }
    };

    class ScopedReadLock {
        private:
            ReaderWriterLock *m_lock;
        public:
            ALWAYS_INLINE ScopedReadLock(ReaderWriterLock *reader_writer_lock) : m_lock(reader_writer_lock) {
                reader_writer_lock->AcquireReadLock();
            }
            ALWAYS_INLINE ~ScopedReadLock() {
                m_lock->ReleaseReadLock();
            }
    };
}
