/* thread_priority.c
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
#include  "thread_priority.h"
#include  "rtos_jcb_ready_list.h"
#include  "rtos_tcb_live_list.h"
#include  "arch4rtos_criticallevel.h"
#include  "nxRTOSConfig.h"

//TCB_t * getCurrentTCB() = NULL;
// {{{ Thread_Priority on currentThreadContext  {{{
UBaseType_t xGetThreadContextPriority(void)
{
    UBaseType_t currentPriority;
    SysCriticalLevel_t origSysCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{ raise SysCriticalLevel to RTOS_SYSCRITICALLEVEL {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    currentPriority = getCurrentTCB()->uxPriority;
    // }}} restore SysCriticalLevel to original }}}
    arch4rtos_iDropSysCriticalLevel(origSysCriticalLevel);

    return  currentPriority;
}

UBaseType_t xSetThreadContextPriority(UBaseType_t  newLevel)
{
    UBaseType_t currentPriority= (UBaseType_t) -1;
    SysCriticalLevel_t origSysCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    if(newLevel < RTOS_HIGHEST_THREAD_PRIORITY)
    {
        newLevel = RTOS_HIGHEST_THREAD_PRIORITY;
    }

    if(newLevel > RTOS_LOWEST_THREAD_PRIORITY)
    {
        newLevel = RTOS_LOWEST_THREAD_PRIORITY;
    }

    // {{{ raise SysCriticalLevel to RTOS_SYSCRITICALLEVEL {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);


    if(getCurrentTCB() != NULL)
    {
        currentPriority = getCurrentTCB()->uxPriority;
        getCurrentTCB()->uxPriority = newLevel;
    }

    if(newLevel > currentPriority)
    {   // when try to lower priority, it need to check more for restriction
        // and may need to invoke reschedule.
        if(getCurrentTCB()->pxJCB != NULL)
        {   // more check with JCB to see if allowed to newLevel
            if (getCurrentTCB()->pxJCB->pThreadStack == ThreadStackTempStacking)
            {
                if(((LiveTCB_t *)getCurrentTCB()->xTcbListItem.prev)->uxPriority
                                                                                                            < newLevel)
                {
                    getCurrentTCB()->uxPriority =
                       ((LiveTCB_t *)getCurrentTCB()->xTcbListItem.prev)->uxPriority;
                }
            }
        }

        //  check and update getCurrentRun2TermTCB() or
        //  getCurrentRun2BlkTCB()
        if(getCurrentRun2BlkTCB() == getCurrentTCB())
        {   // need to check and update getCurrentRun2BlkTCB()
            removeTCBFromRun2BlckTCBList(getCurrentTCB());
            insertTCBToRun2BlckTCBList(getCurrentTCB());
            if(getCurrentRun2BlkTCB() != getCurrentTCB())
            {// after update, if the new getCurrentRun2BlkTCB() changed,
                arch4rtosReqSchedulerService();
            }
        }
        else if (getCurrentRun2TermTCB() == getCurrentTCB())
        {   // need to restrict check for the setting to lower priority
            //getCurrentRun2TermTCB() = (TCB_t *) getCurrentRun2TermTCB()->xTcbListItem.prev;
            LiveTCB_t * thePrevTCB = (LiveTCB_t *)getCurrentTCB()->xTcbListItem.prev;
            if((thePrevTCB != NULL) && (thePrevTCB->uxPriority < getCurrentTCB()->uxPriority))
            {   // reach the limit and have to adjust
                getCurrentTCB()->uxPriority = thePrevTCB->uxPriority;
            }
            if ((getCurrentRun2BlkTCB() != NULL) &&
                    (getCurrentRun2BlkTCB()->uxPriority < getCurrentTCB()->uxPriority))
            {   // the new getCurrentRun2TermTCB() has lower priority
                //  than getCurrentRun2BlkTCB().  reschedule needed.
                arch4rtosReqSchedulerService();
            }
        }
        else
        {   // if the getCurrentTCB() is neither the getCurrentRun2TermTCB() nor
            //  getCurrentRun2BlkTCB(), reschedule needed.
            //  but this could be omitted as this situation is NOT caused by
            //  the new priority setting.
            arch4rtosReqSchedulerService();
        }
    }
    else
    {
        // raise or same priority, no thing to do further
    }
    currentPriority = getCurrentTCB()->uxPriority;
    arch4rtos_iDropSysCriticalLevel(origSysCriticalLevel);
    // }}} restore SysCriticalLevel to original }}}

    return  currentPriority;
}

UBaseType_t xRaiseThreadContextPriority(UBaseType_t  newLevel)
{
    UBaseType_t xOrigPriority, xResultPriority;
    SysCriticalLevel_t origSysCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{ raise SysCriticalLevel to RTOS_SYSCRITICALLEVEL {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    xOrigPriority = xGetThreadContextPriority();
    xResultPriority = xOrigPriority;
    if(newLevel < xOrigPriority)
    {
        xResultPriority = xSetThreadContextPriority(newLevel);
    }
    arch4rtos_iDropSysCriticalLevel(origSysCriticalLevel);
    // }}} restore SysCriticalLevel to original }}}

    return xResultPriority;
}

UBaseType_t xDropThreadContextPriority(UBaseType_t  newLevel)
{
    UBaseType_t xOrigPriority, xResultPriority;
    SysCriticalLevel_t origSysCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{ raise SysCriticalLevel to RTOS_SYSCRITICALLEVEL {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    xOrigPriority = xGetThreadContextPriority();
    xResultPriority = xOrigPriority;
    if(newLevel > xOrigPriority)
    {
        xResultPriority = xSetThreadContextPriority(newLevel);
    }
    arch4rtos_iDropSysCriticalLevel(origSysCriticalLevel);
    // }}} restore SysCriticalLevel to original }}}

    return xResultPriority;
}
// }}}  Thread_Priority on currentThreadContext  }}}

// {{{  Thread_Priority for generic Thread {{{
    // through Job_handle or Thread_handle,  get/set run-time Thread_Priority
    // Lower Thread_Priority to currentThread, may cause RTOS to re-schedule.
    // Raise Thread_Priority to runningThread other than currentThread,
    // may cause RTOS to re-schedule.
    // change Thread_Priority to blocked/suspended liveThread has no impact to
    //  RTOS to scheduling.

UBaseType_t xGetLiveThreadPriority(LiveTCB_t * pLiveThread)
{
    UBaseType_t thePriority = RTOS_LOWEST_THREAD_PRIORITY;
    SysCriticalLevel_t origSysCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{ raise SysCriticalLevel to RTOS_SYSCRITICALLEVEL {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

    if(pLiveThread == NULL)
    {
        pLiveThread = getCurrentTCB();
    }

    if(pLiveThread != NULL)
    {
        thePriority = pLiveThread->uxPriority;
    }
    arch4rtos_iDropSysCriticalLevel(origSysCriticalLevel);
    // }}} restore SysCriticalLevel to original }}}

    return thePriority;
}

UBaseType_t xSetLiveThreadPriority(LiveTCB_t * pLiveThread, UBaseType_t newLevel)
{
    LiveTCB_t * pxSelectToRunTCB;
    UBaseType_t thePriority =RTOS_LOWEST_THREAD_PRIORITY ;
    SysCriticalLevel_t origSysCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    if(newLevel < RTOS_HIGHEST_THREAD_PRIORITY)
    {
        newLevel = RTOS_HIGHEST_THREAD_PRIORITY;
    }

    if(newLevel > RTOS_LOWEST_THREAD_PRIORITY)
    {
        newLevel = RTOS_LOWEST_THREAD_PRIORITY;
    }

    // {{{ raise SysCriticalLevel to RTOS_SYSCRITICALLEVEL {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(pLiveThread == NULL)
    {
        pLiveThread = getCurrentTCB();
    }

    if(pLiveThread != NULL)
    {
        //thePriority = pLiveThread->uxPriority;
        // preliminary set pLiveThread->uxPriority to newLevel
        pLiveThread->uxPriority = newLevel;
        // now need restriction check and fix for some situation
        if ((pLiveThread->pxJCB->pThreadStack == ThreadStackTempStacking) &&
                (pLiveThread->xTcbListItem.prev != NULL))
        {
            if(((LiveTCB_t *)pLiveThread->xTcbListItem.prev)->uxPriority < newLevel)
            {
                pLiveThread->uxPriority =
                                // adjust uxPriority to be same as the one it preempted
                               ((LiveTCB_t *)pLiveThread->xTcbListItem.prev)->uxPriority;
            }
        }

        // calculate pxSelectToRunTCB
        pxSelectToRunTCB = getCurrentRun2TermTCB();
        if(getCurrentRun2TermTCB() ==  NULL)
        {
            pxSelectToRunTCB = getCurrentRun2BlkTCB();
        }
        else if(getCurrentRun2BlkTCB() != NULL)
        {   // neither getCurrentRun2BlkTCB() or getCurrentRun2TermTCB() be NULL
            if(getCurrentRun2TermTCB()->uxPriority > getCurrentRun2BlkTCB()->uxPriority)
            {
                pxSelectToRunTCB = getCurrentRun2BlkTCB();
            }
        }

        //
        if(pLiveThread == getCurrentTCB())
        {   //
            if ((pxSelectToRunTCB != NULL)  &&
                    (pxSelectToRunTCB->uxPriority < getCurrentTCB()->uxPriority))
            {
                arch4rtosReqSchedulerService();
            }

            if((getReadyJCB() != NULL )  &&
                    (getReadyJCB()->uxPriority < getCurrentTCB()->uxPriority))
            {
                arch4rtosReqSchedulerService();
            }
        }
        thePriority = pLiveThread->uxPriority;
    }
    arch4rtos_iDropSysCriticalLevel(origSysCriticalLevel);
    // }}} restore SysCriticalLevel to original }}}

    return thePriority;
}

UBaseType_t xRaiseLiveThreadPriority(LiveTCB_t * pLiveThread, UBaseType_t newLevel)
{
    UBaseType_t thePriority =RTOS_LOWEST_THREAD_PRIORITY ;
    SysCriticalLevel_t origSysCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    if(newLevel < RTOS_HIGHEST_THREAD_PRIORITY)
    {
        newLevel = RTOS_HIGHEST_THREAD_PRIORITY;
    }

    if(newLevel > RTOS_LOWEST_THREAD_PRIORITY)
    {
        newLevel = RTOS_LOWEST_THREAD_PRIORITY;
    }

    // {{{ raise SysCriticalLevel to RTOS_SYSCRITICALLEVEL {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(pLiveThread == NULL)
    {
        pLiveThread = getCurrentTCB();
    }

    if(pLiveThread != NULL)
    {
        if(pLiveThread->uxPriority > newLevel)
        {
            thePriority = xSetLiveThreadPriority(pLiveThread, newLevel);
        }
        else
        {
            thePriority = xGetLiveThreadPriority(pLiveThread);
        }
    }

    arch4rtos_iDropSysCriticalLevel(origSysCriticalLevel);
    // }}} restore SysCriticalLevel to original }}}

    return  thePriority;
}

UBaseType_t xDropLiveThreadPriority(LiveTCB_t * pLiveThread, UBaseType_t newLevel)
{
    UBaseType_t thePriority =RTOS_LOWEST_THREAD_PRIORITY ;
    SysCriticalLevel_t origSysCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    if(newLevel < RTOS_HIGHEST_THREAD_PRIORITY)
    {
        newLevel = RTOS_HIGHEST_THREAD_PRIORITY;
    }

    if(newLevel > RTOS_LOWEST_THREAD_PRIORITY)
    {
        newLevel = RTOS_LOWEST_THREAD_PRIORITY;
    }

    // {{{ raise SysCriticalLevel to RTOS_SYSCRITICALLEVEL {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(pLiveThread == NULL)
    {
        pLiveThread = getCurrentTCB();
    }

    if(pLiveThread != NULL)
    {
        if(pLiveThread->uxPriority < newLevel)
        {
            thePriority = xSetLiveThreadPriority(pLiveThread, newLevel);
        }
        else
        {
            thePriority = xGetLiveThreadPriority(pLiveThread);
        }
    }

    arch4rtos_iDropSysCriticalLevel(origSysCriticalLevel);
    // }}} restore SysCriticalLevel to original }}}

    return  thePriority;
}
// }}}  Thread_Priority for running Job/Thread  }}}

/// {{{ TEST_ThreadPriority {{{
#if     defined(TEST_ThreadPriority)
// there are restrictions to xSetLiveThreadPriority in a StackShared type of Thread_context
// it should be OK to invoke from FirstJob_Handler before any other Job
// committed into RTOS kernel.

int xTestThreadPriority()
{
    UBaseType_t origPriority = xGetLiveThreadPriority(NULL);

    for(int i = RTOS_HIGHEST_THREAD_PRIORITY; i <= RTOS_LOWEST_THREAD_PRIORITY; i++)
    {
        if(i != xSetLiveThreadPriority(NULL, i))
        {
            while(1);
        }

        if(i != xGetLiveThreadPriority(NULL))
        {
            while(1);
        }
    }

    xSetLiveThreadPriority(NULL, RTOS_LOWEST_THREAD_PRIORITY);
    for(int i = RTOS_LOWEST_THREAD_PRIORITY; i <= RTOS_HIGHEST_THREAD_PRIORITY; i--)
    {
        if(i != xRaiseLiveThreadPriority(NULL, i))
        {
            while(1);
        }

        if(i != xGetLiveThreadPriority(NULL))
        {
            while(1);
        }
    }

    for(int i = RTOS_HIGHEST_THREAD_PRIORITY; i <= RTOS_LOWEST_THREAD_PRIORITY; i++)
    {
        if(RTOS_HIGHEST_THREAD_PRIORITY != xRaiseLiveThreadPriority(NULL, i))
        {
            while(1);
        }

        if(RTOS_HIGHEST_THREAD_PRIORITY != xGetLiveThreadPriority(NULL))
        {
            while(1);
        }
    }

    for(int i = RTOS_HIGHEST_THREAD_PRIORITY; i <= RTOS_LOWEST_THREAD_PRIORITY; i++)
    {
        if(i != xDropLiveThreadPriority(NULL, i))
        {
            while(1);
        }

        if(i != xGetLiveThreadPriority(NULL))
        {
            while(1);
        }
    }

    for(int i = RTOS_HIGHEST_THREAD_PRIORITY; i <= RTOS_LOWEST_THREAD_PRIORITY; i++)
    {
        if(RTOS_LOWEST_THREAD_PRIORITY != xDropLiveThreadPriority(NULL, i))
        {
            while(1);
        }

        if(RTOS_LOWEST_THREAD_PRIORITY != xGetLiveThreadPriority(NULL))
        {
            while(1);
        }
    }


    xSetLiveThreadPriority(NULL, origPriority);
    return 0;
}

#endif
/// }}} TEST_ThreadPriority }}}




