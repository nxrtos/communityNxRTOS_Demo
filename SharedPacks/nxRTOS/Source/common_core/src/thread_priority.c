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
#include  "list_jcb.h"
#include  "list_tcb.h"
#include  "arch4rtos_criticallevel.h"
#include  "nxRTOSConfig.h"

//TCB_t * pxCurrentTCB = NULL;
// {{{ Thread_Priority on currentThreadContext  {{{
UBaseType_t xGetThreadContextPriority(void)
{
    UBaseType_t currentPriority;
    SysCriticalLevel_t origSysCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{ raise SysCriticalLevel to RTOS_SYSCRITICALLEVEL {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    currentPriority = pxCurrentTCB->uxPriority;
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


    if(pxCurrentTCB != NULL)
    {
        currentPriority = pxCurrentTCB->uxPriority;
        pxCurrentTCB->uxPriority = newLevel;
    }

    if(newLevel > currentPriority)
    {   // when try to lower priority, it need to check more for restriction
        // and may need to invoke reschedule.
        if(pxCurrentTCB->pxJCB != NULL)
        {   // more check with JCB to see if allowed to newLevel
            if (pxCurrentTCB->pxJCB->pThreadStack == ThreadStackTempStacking)
            {
                if(((BaseTCB_t *)pxCurrentTCB->xStateListItem.prev)->uxPriority
                                                                                                            < newLevel)
                {
                    pxCurrentTCB->uxPriority =
                       ((BaseTCB_t *)pxCurrentTCB->xStateListItem.prev)->uxPriority;
                }
            }
        }

        //  check and update pxRun2TermTCBList_Head or
        //  pxRun2BlckTCBList_Head
        if(pxRun2BlckTCBList_Head == pxCurrentTCB)
        {   // need to check and update pxRun2BlckTCBList_Head
            pickTCBFromRun2BlckTCBList(pxCurrentTCB);
            addTCBToRun2BlckTCBList(pxCurrentTCB);
            if(pxRun2BlckTCBList_Head != pxCurrentTCB)
            {// after update, if the new pxRun2BlckTCBList_Head changed,
                arch4rtosReqSchedulerService();
            }
        }
        else if (pxRun2TermTCBList_Head == pxCurrentTCB)
        {   // need to restrict check for the setting to lower priority
            //pxRun2TermTCBList_Head = (TCB_t *) pxRun2TermTCBList_Head->xStateListItem.prev;
            BaseTCB_t * thePrevTCB = (BaseTCB_t *)pxCurrentTCB->xStateListItem.prev;
            if((thePrevTCB != NULL) && (thePrevTCB->uxPriority < pxCurrentTCB->uxPriority))
            {   // reach the limit and have to adjust
                pxCurrentTCB->uxPriority = thePrevTCB->uxPriority;
            }
            if ((pxRun2BlckTCBList_Head != NULL) &&
                    (pxRun2BlckTCBList_Head->uxPriority < pxCurrentTCB->uxPriority))
            {   // the new pxRun2TermTCBList_Head has lower priority
                //  than pxRun2BlckTCBList_Head.  reschedule needed.
                arch4rtosReqSchedulerService();
            }
        }
        else
        {   // if the pxCurrentTCB is neither the pxRun2TermTCBList_Head nor
            //  pxRun2BlckTCBList_Head, reschedule needed.
            //  but this could be omitted as this situation is NOT caused by
            //  the new priority setting.
            arch4rtosReqSchedulerService();
        }
    }
    else
    {
        // raise or same priority, no thing to do further
    }
    currentPriority = pxCurrentTCB->uxPriority;
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

UBaseType_t xGetLiveThreadPriority(BaseTCB_t * pLiveThread)
{
    UBaseType_t thePriority = RTOS_LOWEST_THREAD_PRIORITY;
    SysCriticalLevel_t origSysCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{ raise SysCriticalLevel to RTOS_SYSCRITICALLEVEL {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

    if(pLiveThread == NULL)
    {
        pLiveThread = pxCurrentTCB;
    }

    if(pLiveThread != NULL)
    {
        thePriority = pLiveThread->uxPriority;
    }
    arch4rtos_iDropSysCriticalLevel(origSysCriticalLevel);
    // }}} restore SysCriticalLevel to original }}}

    return thePriority;
}

UBaseType_t xSetLiveThreadPriority(BaseTCB_t * pLiveThread, UBaseType_t newLevel)
{
    BaseTCB_t * pxSelectToRunTCB;
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
        pLiveThread = pxCurrentTCB;
    }

    if(pLiveThread != NULL)
    {
        //thePriority = pLiveThread->uxPriority;
        // preliminary set pLiveThread->uxPriority to newLevel
        pLiveThread->uxPriority = newLevel;
        // now need restriction check and fix for some situation
        if ((pLiveThread->pxJCB->pThreadStack == ThreadStackTempStacking) &&
                (pLiveThread->xStateListItem.prev != NULL))
        {
            if(((BaseTCB_t *)pLiveThread->xStateListItem.prev)->uxPriority < newLevel)
            {
                pLiveThread->uxPriority =
                                // adjust uxPriority to be same as the one it preempted
                               ((BaseTCB_t *)pLiveThread->xStateListItem.prev)->uxPriority;
            }
        }

        // calculate pxSelectToRunTCB
        pxSelectToRunTCB = pxRun2TermTCBList_Head;
        if(pxRun2TermTCBList_Head ==  NULL)
        {
            pxSelectToRunTCB = pxRun2BlckTCBList_Head;
        }
        else if(pxRun2BlckTCBList_Head != NULL)
        {   // neither pxRun2BlckTCBList_Head or pxRun2TermTCBList_Head be NULL
            if(pxRun2TermTCBList_Head->uxPriority > pxRun2BlckTCBList_Head->uxPriority)
            {
                pxSelectToRunTCB = pxRun2BlckTCBList_Head;
            }
        }

        //
        if(pLiveThread == pxCurrentTCB)
        {   //
            if ((pxSelectToRunTCB != NULL)  &&
                    (pxSelectToRunTCB->uxPriority < pxCurrentTCB->uxPriority))
            {
                arch4rtosReqSchedulerService();
            }

            if((pxReadyListJCB != NULL )  &&
                    (pxReadyListJCB->uxPriority < pxCurrentTCB->uxPriority))
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

UBaseType_t xRaiseLiveThreadPriority(BaseTCB_t * pLiveThread, UBaseType_t newLevel)
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
        pLiveThread = pxCurrentTCB;
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

UBaseType_t xDropLiveThreadPriority(BaseTCB_t * pLiveThread, UBaseType_t newLevel)
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
        pLiveThread = pxCurrentTCB;
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




