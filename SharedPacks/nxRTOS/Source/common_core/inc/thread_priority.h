/* thread_priority.h
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * 1 tab == 4 spaces!
 */

/*-----------------------------------------------------------------------------
 * Thread is program running in user space context, starting from a ready JCB
 * been transferring into executing.
 * Each thread comes with a ThreadControlBlock, which is located in the bottom
 * of stack of the thread.
 *
 * currentRunningThreadP, point to the current running thread's TCB, this
 * will be the highest priority of thread among other running threads and
 * the priority also be no less than readyJCBHead's priority.
 *
 * a lower priority of Thread may preempted by a higher priority of running
 * Thread.
 *
 * When a Thread is type of run2TermThread, it will be treated in a special
 * way by scheduler. As all run2TermThreads's stacks will be stacked up to
 * share a chunk of RAM space, with lower priority of thread in the bottom
 * and highest priority of thread on the top. The thread/stack in the bottom
 * is frozen until the threads on the top all went terminated.
 * As result, there is run2TermThreadList_Head point to a run2TermThread with
 * highest priority among other run2TermThreads. The Thread pointed by
 * run2TermThreadList_Head can increase its own priority to highest Thread
 * priority and decrease to the no lower than its immediate preempted
 * run2TermThread.
 *
 * When a Thread is type of run2BlckThread, it will has an exclusive RAM
 * chunk assigned for stack space of run2BlckThread. When run2BlckThread
 * reach a blocking point to be suspended, the TCB will be moved to a List
 * holding by the corresponding RTOS object (aka Semaphore, Mutex, SysTimer
 * etc). If a run2BlckThread is preempted, it will be kept in run2BlckThreadList.
 *
 * currentRunningThreadP will have the same value to one of run2TermThread_Head
 * or run2BlckThread_Head.
 *
 * A special Init_Thread is a type of run2TermThread, started by startRTOS()
 * through SysInitJob.  At the end of Init_Thread, it need to commit a
 * repeating Idle_Processig_Job, which is causing Idle_run2TermThread to
 * executed in exclusive lowest priority repeatedly when no other running
 * Thread or ready Job available.
 *
 *---------------------------------------------------------------------------*/

#ifndef THREAD_PRIORITY_H
#define THREAD_PRIORITY_H

#include  "rtos_tcb_live.h"
//#include  "arch_basetype.h"

#ifdef __cplusplus
extern "C" {
#endif

//TCB_t * getCurrentTCB() = NULL;
// {{{ Thread_Priority on currentThreadContext  {{{
extern  UBaseType_t xGetThreadContextPriority(void);
extern  UBaseType_t xSetThreadContextPriority(UBaseType_t  newLevel);
extern  UBaseType_t xRaiseThreadContextPriority(UBaseType_t  newLevel);
extern  UBaseType_t xDropThreadContextPriority(UBaseType_t  newLevel);
// }}}  Thread_Priority on currentThreadContext  }}}

// {{{  Thread_Priority for generic Thread {{{
    // through Job_handle or Thread_handle,  get/set run-time Thread_Priority
    // Lower Thread_Priority to currentThread, may cause RTOS to re-schedule.
    // Raise Thread_Priority to runningThread other than currentThread,
    // may cause RTOS to re-schedule.
    // change Thread_Priority to blocked/suspended liveThread has no impact to
    //  RTOS to scheduling.

extern  UBaseType_t xGetLiveThreadPriority(LiveTCB_t * pLiveThread);
extern  UBaseType_t xSetLiveThreadPriority(LiveTCB_t * pLiveThread, UBaseType_t newLevel);
extern  UBaseType_t xRaiseLiveThreadPriority(LiveTCB_t * pLiveThread, UBaseType_t newLevel);

extern  UBaseType_t xDropLiveThreadPriority(LiveTCB_t * pLiveThread, UBaseType_t newLevel);
// }}}  Thread_Priority for running Job/Thread  }}}

#define     TEST_ThreadPriority     1
// there are restrictions to xSetLiveThreadPriority in a StackShared type of Thread_context
// it should be OK to invoke from FirstJob_Handler before any other Job
// committed into RTOS kernel.
#if     defined(TEST_ThreadPriority)
extern  int xTestThreadPriority(void);
#endif

#ifdef __cplusplus
}
#endif

#endif  //THREAD_PRIORITY_H
