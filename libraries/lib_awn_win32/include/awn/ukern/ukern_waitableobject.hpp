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

namespace awn::ukern::impl {

    class WaitableObject {
        public:
            constexpr WaitableObject() {/*...*/}

            virtual void EndWait(FiberLocalStorage *wait_fiber, Result wait_result) = 0;
            virtual void CancelWait(FiberLocalStorage *wait_fiber, Result wait_result) = 0;

            void EndFiberWaitImpl(FiberLocalStorage *wait_fiber, Result wait_result) {

                /* Remove from suspend/wait list */
                wait_fiber->scheduler_list_node.Unlink();

                /* Set Fiber state */
                wait_fiber->fiber_state = FiberState_Scheduled;
                wait_fiber->last_result = wait_result;
                wait_fiber->timeout     = 0;

                /* Add to scheduler */
                GetScheduler()->AddToSchedulerUnsafe(wait_fiber);
            }
    };

    class LockArbiter : public WaitableObject {
        public:
            constexpr LockArbiter() {/*...*/}

            virtual void EndWait(FiberLocalStorage *wait_fiber, Result wait_result) override {
                EndFiberWaitImpl(wait_fiber, wait_result);
            }

            virtual void CancelWait(FiberLocalStorage *wait_fiber, [[maybe_unused]] Result wait_result) override {
                VP_ASSERT(false);
                /* Remove from child list */
                wait_fiber->ReleaseLockWaitListUnsafe();
            }
    };

    class KeyArbiter : public WaitableObject {
        public:
            constexpr KeyArbiter() {/*...*/}

            virtual void EndWait(FiberLocalStorage *wait_fiber, Result wait_result) override {
                EndFiberWaitImpl(wait_fiber, wait_result);
            }

            virtual void CancelWait(FiberLocalStorage *wait_fiber, Result wait_result) override {

                /* Transfer wait list or remove from child list */
                if (wait_fiber->wait_list.IsEmpty() == false) {

                    FiberLocalStorage *next_cv_parent = std::addressof(wait_fiber->wait_list.PopFront());

                    FiberLocalStorage::WaitList::iterator wait_list_iter = wait_fiber->wait_list.begin();
                    while (wait_list_iter != wait_fiber->wait_list.end()) {

                        /* Pre-iterate for list removal */
                        FiberLocalStorage &waiting_fiber = *wait_list_iter;
                        ++wait_list_iter;

                        /* Detach from previous list */
                        waiting_fiber.wait_list_node.Unlink();

                        /* Add to new parent */
                        next_cv_parent->wait_list.PushBack(waiting_fiber);
                    }
                } else if (wait_fiber->wait_list_node.IsLinked() == true) {
                    wait_fiber->wait_list_node.Unlink();
                } else {
                    EndFiberWaitImpl(wait_fiber, wait_result);
                    return;
                }

                /* Try to take the lock back */
                const u32 prev_tag = *wait_fiber->lock_address;
                u32       tag      =  wait_fiber->wait_tag;
                if (prev_tag != 0) {
                    tag |= FiberLocalStorage::HasChildWaitersBit;
                }
                *wait_fiber->lock_address = tag;

                /* If there were other waiters */
                if (prev_tag != 0) {
                    /* Get fiber by handle */
                    FiberLocalStorage *lock_fiber = impl::GetScheduler()->GetFiberByHandle(prev_tag & ~FiberLocalStorage::HasChildWaitersBit);

                    /* Push back fiber waiter */
                    lock_fiber->wait_list.PushBack(*wait_fiber);

                    /* Swap to suspend list */
                    lock_fiber->scheduler_list_node.Unlink();
                    impl::GetScheduler()->m_suspended_list.PushBack(*lock_fiber);

                } else {
                    EndFiberWaitImpl(wait_fiber, wait_result);
                }

                return;
            }
    };

    class WaitAddressArbiter : public WaitableObject {
        public:
            constexpr WaitAddressArbiter() {/*...*/}

            virtual void EndWait(FiberLocalStorage *wait_fiber, Result wait_result) override {
                EndFiberWaitImpl(wait_fiber, wait_result);
            }

            virtual void CancelWait(FiberLocalStorage *wait_fiber, Result wait_result) override {

                /* Transfer wait list or remove from child list */
                if (wait_fiber->wait_list.IsEmpty() == false) {

                    FiberLocalStorage *next_wait_parent = std::addressof(wait_fiber->wait_list.PopFront());

                    FiberLocalStorage::WaitList::iterator wait_list_iter = wait_fiber->wait_list.begin();
                    while (wait_list_iter != wait_fiber->wait_list.end()) {

                        /* Pre-iterate for list removal */
                        FiberLocalStorage &waiting_fiber = *wait_list_iter;
                        ++wait_list_iter;

                        /* Detach from previous list */
                        waiting_fiber.wait_list_node.Unlink();

                        /* Add to new parent */
                        next_wait_parent->wait_list.PushBack(waiting_fiber);
                    }
                } else if (wait_fiber->wait_list_node.IsLinked() == true) {
                    wait_fiber->wait_list_node.Unlink();
                }

                EndFiberWaitImpl(wait_fiber, wait_result);
                return;
            }
    };
}
