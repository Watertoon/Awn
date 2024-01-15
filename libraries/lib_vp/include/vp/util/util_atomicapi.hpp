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

    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedLoad(T *read) {
        return __atomic_load_n(read, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedLoadAcquire(T *read) {
        return __atomic_load_n(read, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedLoadRelease(T *read) {
        return __atomic_load_n(read, __ATOMIC_RELEASE);
    }

    template <typename T>
    constexpr ALWAYS_INLINE void InterlockedStore(T *write, T value) {
        __atomic_store_n(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE void InterlockedStoreAcquire(T *write, T value) {
        __atomic_store_n(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE void InterlockedStoreRelease(T *write, T value) {
        __atomic_store_n(write, value, __ATOMIC_RELEASE);
    }

    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedExchange(T *write, T value) {
        return __atomic_exchange_n(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedExchangeAcquire(T *write, T value) {
        return __atomic_exchange_n(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedExchangeRelease(T *write, T value) {
        return __atomic_exchange_n(write, value, __ATOMIC_RELEASE);
    }

    template <typename T>
    constexpr ALWAYS_INLINE bool InterlockedCompareExchange(T *out_last, T *write, T value, T comperand) {
        const bool result = __atomic_compare_exchange_n(write, std::addressof(comperand), value, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        if (out_last != nullptr) { *out_last = comperand; }
        return result;
    }
    template <typename T>
    constexpr ALWAYS_INLINE bool InterlockedCompareExchangeAcquire(T *out_last, T *write, T value, T comperand) {
        const bool result = __atomic_compare_exchange_n(write, std::addressof(comperand), value, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
        if (out_last != nullptr) { *out_last = comperand; }
        return result;
    }
    template <typename T>
    constexpr ALWAYS_INLINE bool InterlockedCompareExchangeRelease(T *out_last, T *write, T value, T comperand) {
        const bool result = __atomic_compare_exchange_n(write, std::addressof(comperand), value, false, __ATOMIC_RELEASE, __ATOMIC_RELAXED);
        if (out_last != nullptr) { *out_last = comperand; }
        return result;
    }
    template <typename T>
    constexpr ALWAYS_INLINE bool InterlockedCompareExchangeRelaxed(T *out_last, T *write, T value, T comperand) {
        const bool result = __atomic_compare_exchange_n(write, std::addressof(comperand), value, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
        if (out_last != nullptr) { *out_last = comperand; }
        return result;
    }
    
    
    template <typename T>
    constexpr ALWAYS_INLINE bool InterlockedCompareExchangeWeak(T *out_last, T *write, T value, T comperand) {
        const bool result = __atomic_compare_exchange_n(write, std::addressof(comperand), value, true, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        if (out_last != nullptr) { *out_last = comperand; }
        return result;
    }
    template <typename T>
    constexpr ALWAYS_INLINE bool InterlockedCompareExchangeWeakAcquire(T *out_last, T *write, T value, T comperand) {
        const bool result = __atomic_compare_exchange_n(write, std::addressof(comperand), value, true, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
        if (out_last != nullptr) { *out_last = comperand; }
        return result;
    }
    template <typename T>
    constexpr ALWAYS_INLINE bool InterlockedCompareExchangeWeakRelease(T *out_last, T *write, T value, T comperand) {
        const bool result = __atomic_compare_exchange_n(write, std::addressof(comperand), value, true, __ATOMIC_RELEASE, __ATOMIC_RELAXED);
        if (out_last != nullptr) { *out_last = comperand; }
        return result;
    }

    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedAdd(T *write, T value) {
        return __atomic_add_fetch(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedAddAcquire(T *write, T value) {
        return __atomic_add_fetch(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedAddRelease(T *write, T value) {
        return __atomic_add_fetch(write, value, __ATOMIC_RELEASE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchAdd(T *write, T value) {
        return __atomic_fetch_add(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchAddAcquire(T *write, T value) {
        return __atomic_fetch_add(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchAddRelease(T *write, T value) {
        return __atomic_fetch_add(write, value, __ATOMIC_RELEASE);
    }

    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedIncrement(T *write) {
        return __atomic_add_fetch(write, 1, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedIncrementAcquire(T *write) {
        return __atomic_add_fetch(write, 1, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedIncrementRelease(T *write) {
        return __atomic_add_fetch(write, 1, __ATOMIC_RELEASE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchIncrement(T *write) {
        return __atomic_fetch_add(write, 1, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchIncrementAcquire(T *write) {
        return __atomic_fetch_add(write, 1, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchIncrementRelease(T *write) {
        return __atomic_fetch_add(write, 1, __ATOMIC_RELEASE);
    }

    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedSubtract(T *write, T value) {
        return __atomic_sub_fetch(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedSubtractAcquire(T *write, T value) {
        return __atomic_sub_fetch(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedSubtractRelease(T *write, T value) {
        return __atomic_sub_fetch(write, value, __ATOMIC_RELEASE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchSubtract(T *write, T value) {
        return __atomic_fetch_sub(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchSubtractAcquire(T *write, T value) {
        return __atomic_fetch_sub(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchSubtractRelease(T *write, T value) {
        return __atomic_fetch_sub(write, value, __ATOMIC_RELEASE);
    }

    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedDecrement(T *write) {
        return __atomic_sub_fetch(write, 1, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedDecrementAcquire(T *write) {
        return __atomic_sub_fetch(write, 1, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedDecrementRelease(T *write) {
        return __atomic_sub_fetch(write, 1, __ATOMIC_RELEASE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchDecrement(T *write) {
        return __atomic_fetch_sub(write, 1, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchDecrementAcquire(T *write) {
        return __atomic_fetch_sub(write, 1, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchDecrementRelease(T *write) {
        return __atomic_fetch_sub(write, 1, __ATOMIC_RELEASE);
    }

    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedAnd(T *write, T value) {
        return __atomic_and_fetch(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedAndAcquire(T *write, T value) {
        return __atomic_and_fetch(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedAndRelease(T *write, T value) {
        return __atomic_and_fetch(write, value, __ATOMIC_RELEASE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchAnd(T *write, T value) {
        return __atomic_fetch_and(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchAndAcquire(T *write, T value) {
        return __atomic_fetch_and(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchAndRelease(T *write, T value) {
        return __atomic_fetch_and(write, value, __ATOMIC_RELEASE);
    }

    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedXor(T *write, T value) {
        return __atomic_xor_fetch(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedXorAcquire(T *write, T value) {
        return __atomic_xor_fetch(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedXorRelease(T *write, T value) {
        return __atomic_xor_fetch(write, value, __ATOMIC_RELEASE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchXor(T *write, T value) {
        return __atomic_fetch_xor(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchXorAcquire(T *write, T value) {
        return __atomic_fetch_xor(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchXorRelease(T *write, T value) {
        return __atomic_fetch_xor(write, value, __ATOMIC_RELEASE);
    }

    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedOr(T *write, T value) {
        return __atomic_or_fetch(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedOrAcquire(T *write, T value) {
        return __atomic_or_fetch(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedOrRelease(T *write, T value) {
        return __atomic_or_fetch(write, value, __ATOMIC_RELEASE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchOr(T *write, T value) {
        return __atomic_fetch_or(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchOrAcquire(T *write, T value) {
        return __atomic_fetch_or(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchOrRelease(T *write, T value) {
        return __atomic_fetch_or(write, value, __ATOMIC_RELEASE);
    }

    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedNand(T *write, T value) {
        return __atomic_nand_fetch(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedNandAcquire(T *write, T value) {
        return __atomic_nand_fetch(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedNandRelease(T *write, T value) {
        return __atomic_nand_fetch(write, value, __ATOMIC_RELEASE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchNand(T *write, T value) {
        return __atomic_fetch_nand(write, value, __ATOMIC_SEQ_CST);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchNandAcquire(T *write, T value) {
        return __atomic_fetch_nand(write, value, __ATOMIC_ACQUIRE);
    }
    template <typename T>
    constexpr ALWAYS_INLINE T InterlockedFetchNandRelease(T *write, T value) {
        return __atomic_fetch_nand(write, value, __ATOMIC_RELEASE);
    }
    
    
    constexpr ALWAYS_INLINE void MemoryBarrierAcquire() {
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
    }
    constexpr ALWAYS_INLINE void MemoryBarrierRelease() {
        __atomic_thread_fence(__ATOMIC_RELEASE);
    }
}
