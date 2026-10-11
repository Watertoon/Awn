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

namespace awn::mem {

    class FrameHeap : public Heap {
        protected:
            static constexpr size_t cMinimumAllocationGranularity = sizeof(size_t);
        protected:
            uintptr_t m_iter;
            uintptr_t m_end;
        public:
            VP_RTTI_DERIVED(FrameHeap);
        public:
            FrameHeap(const char *name, Heap *parent_heap, void *start_address, size_t size, bool is_thread_safe) : Heap(name, parent_heap, start_address, size, is_thread_safe), m_iter() {/*...*/}
            virtual ~FrameHeap() override {/*...*/}

            static FrameHeap *TryCreate(const char *name, mem::Heap *parent_heap, size_t size, s32 alignment, bool is_thread_safe);

            virtual MemoryRange AdjustHeap() override;

            virtual void *TryAllocate(size_t size, s32 alignment) override;
            virtual void FreeAll() override;

            virtual size_t GetTotalFreeSize() override;
            virtual size_t GetMaximumAllocatableSize(s32 alignment) override;

            virtual bool IsEmpty() const override;
    };

    //class GpuFrameHeap : public FrameHeap {
    //    public:
    //        GpuFrameHeap(const char *name, Heap *parent_heap, void *start_address, size_t size, bool is_thread_safe) : FrameHeap(name, parent_heap, start_address, size, is_thread_safe) {/*...*/}
    //        virtual ~GpuFrameHeap() override {/*...*/}
    //
    //        static GpuFrameHeap *TryCreate(const char *name, mem::Heap *cpu_heap, mem::Heap *gpu_heap, size_t size, s32 alignment, bool is_thread_safe);
    //
    //        virtual void FreeAll() override;
    //
    //        virtual constexpr bool IsGpuHeap() const override { return true; }
    //        virtual bool IsEmpty() const override { return m_iter == reinterpret_cast<uintptr_t>(m_start_address); }
    //};
}
