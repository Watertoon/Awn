#pragma once

namespace vp::mem {

    struct UnitHeapMemoryBlock {
        UnitHeapMemoryBlock *next;
    };
    class UnitHeap : public Heap {
        private:
            size_t m_block_size;
            UnitHeapMemoryBlock *m_free_block_list;
        public:
            constexpr UnitHeap() {/*...*/}

            virtual void Finalize() override {/*...*/}

            virtual void Adjust() override {/*...*/}

            virtual void *TryAllocate(size_t size, size_t alignment) override {
                UnitHeapMemoryBlock *block = m_free_block_list;
                if (block != nullptr) {
                    m_free_block_list = *m_free_block_list;
                }
                return block;
            }

            virtual void Free(void *address) override {
                if (address != nullptr) {
                    *reinterpret_cast<UnitHeapMemoryBlock**>(address) = m_free_block_list;
                    m_free_block_list = reinterpret_cast<UnitHeapMemoryBlock*>(address);
                }
            }

            virtual void ResizeFront() override {/*...*/}

            virtual void ResizeBack() override {/*...*/}

            virtual size_t GetTotalSize() override {/*...*/}
            virtual size_t GetFreeSize() override {/*...*/}
            virtual size_t GetMaximumAllocatableSize() override {/*...*/}
    };
}
