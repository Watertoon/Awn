#pragma once

namespace awn::res {

    struct LocalArchiveBinder {
        ResourceBinder resource_binder;
        u32            reference_count;
    };

	class ThreadLocalArchiveManager {
        public:
            struct ArchiveTreeNode {
                vp::util::IntrusiveRedBlackTreeNode<sys::ThreadBase*>  m_tree_node;
                LocalArchiveBinder                                    *m_local_archive_binder;

                constexpr ArchiveTreeNode(LocalArchiveBinder *binder) : m_tree_node(), m_local_archive_binder(binder) {}
                constexpr ~ArchiveTreeNode() {}
            };
        public:
            using ThreadRing              = vp::util::RingBuffer<sys::ThreadBase*>;
            using ArchiveTreeMapAllocator = vp::util::IntrusiveRedBlackTreeAllocator<vp::util::IntrusiveRedBlackTreeTraits<ArchiveTreeNode, &ArchiveTreeNode::m_tree_node>>;
            using ArchiveBinderRing       = vp::util::RingBuffer<LocalArchiveBinder*>;
            using ArchiveBinderArray      = vp::util::HeapArray<LocalArchiveBinder>;
		private:
            u32                         m_is_trigger_clear_tree;
            sys::TlsSlot                m_tls_slot;
            sys::ServiceCriticalSection m_cs;
			ArchiveTreeMapAllocator     m_archive_tree_allocator;
            ArchiveBinderRing           m_free_archive_ring_buffer;
            ArchiveBinderArray          m_archive_binder_array;
            ThreadRing                  m_registered_thread_ring;
        private:
            static void TlsDestructor(void *arg);
		public:
			constexpr  ThreadLocalArchiveManager() : m_tls_slot(sys::cInvalidTlsSlot), m_cs(), m_archive_tree_allocator(), m_free_archive_ring_buffer(), m_archive_binder_array(), m_registered_thread_ring() {/*...*/}
			constexpr ~ThreadLocalArchiveManager() {/*...*/}

            void Initialize(mem::Heap *heap, u32 max_thread_count);
            void Finalize();

            void Calculate();

			LocalArchiveBinder *GetThreadLocalArchive();
			ResourceBinder     *GetThreadLocalArchiveBinder();
    
            LocalArchiveBinder *RegisterThread(sys::ThreadBase *thread);

            void UnregisterThread(sys::ThreadBase *thread);
            void UnregisterCurrentThread();

			bool SetThreadLocalArchive(ResourceBinder *binder_to_reference);

            bool IsThreadLocalArchiveInReference();
            
			void ReferenceThreadLocalArchive();

			void ReleaseThreadLocalArchive();
	};
    
    class ScopedThreadLocalArchive {
        private:
            ResourceBinder m_last_binder;
        public:
            ScopedThreadLocalArchive(ResourceBinder *archive_binder);
            ~ScopedThreadLocalArchive();
    };
}
