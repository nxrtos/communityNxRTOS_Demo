/* list_tcb.c
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

#include  "list_tcb.h"
#include  "nxRTOSConfig.h"

BaseTCB_t * pxCurrentTCB = NULL;

/// {{{ run2TermListTCB {{{
BaseTCB_t * pxRun2TermTCBList_Head = NULL;

/// }}} run2TermListTCB }}}


/// {{{ run2BlckListTCB {{{
BaseTCB_t * pxRun2BlckTCBList_Head = NULL;

/// }}} run2BlckListTCB }}}


// in Run2XxxxTCBList, ->xStateListItem.prev point to lower priority of TCB


BaseTCB_t * pickTCBFromRun2BlckTCBList(BaseTCB_t * theTCB)
{
    if(theTCB != NULL)
    {
        // TODO, more check to make sure theTCB is in the Run2BlckTCBList
        BaseTCB_t * refTCB;

        refTCB = (BaseTCB_t *)theTCB->xStateListItem.prev;
        if(refTCB != NULL)
        {
            refTCB->xStateListItem.next = theTCB->xStateListItem.next;
        }

        refTCB = (BaseTCB_t *)theTCB->xStateListItem.next;
        if(refTCB != NULL)
        {
            refTCB->xStateListItem.prev = theTCB->xStateListItem.prev;
        }
        else
        {   // this means theTCB is the HEAD
            if(theTCB != pxRun2BlckTCBList_Head)
            {
                while(1);
            }
            else
            {   // update new pxRun2BlckTCBList_Head
                pxRun2BlckTCBList_Head = (BaseTCB_t *)theTCB->xStateListItem.prev;
            }
        }
    }
    return theTCB;
}

BaseTCB_t * addTCBToRun2BlckTCBList(BaseTCB_t * theTCB)
{
    if(theTCB != NULL)
    {
        if(pxRun2BlckTCBList_Head == NULL)
        {
            pxRun2BlckTCBList_Head = theTCB;
            theTCB->xStateListItem.next = NULL;
            theTCB->xStateListItem.prev = NULL;
        }
        else
        {
            BaseTCB_t * refTCB;
            refTCB = pxRun2BlckTCBList_Head;
            do
            {
                if(refTCB->uxPriority > theTCB->uxPriority)
                {   // add theTCB before refTCB
                    theTCB->xStateListItem.next = refTCB->xStateListItem.next;
                    theTCB->xStateListItem.prev = (ListXItem_t *)refTCB;
                    refTCB->xStateListItem.next = (ListXItem_t *)theTCB;
                    if(refTCB == pxRun2BlckTCBList_Head)
                    {   // new pxRun2BlckTCBList_Head
                        pxRun2BlckTCBList_Head = theTCB;
                    }
                    if(theTCB->xStateListItem.next != NULL)
                    {
                        refTCB = (BaseTCB_t *)theTCB->xStateListItem.next;
                        refTCB->xStateListItem.prev = (ListXItem_t *)theTCB;
                    }
                    break;
                }
                else        // if(refTCB->uxPriority <= theTCB->uxPriority)
                {   // move refTCB to .prev
                    if(refTCB->xStateListItem.prev != NULL)
                    {
                        refTCB = (BaseTCB_t *)refTCB->xStateListItem.prev;
                    }
                    else
                    {   // add theTCB after refTCB aka the last in the RunXxxList
                        theTCB->xStateListItem.next = (ListXItem_t *)refTCB;
                        theTCB->xStateListItem.prev = refTCB->xStateListItem.prev; // NULL
                        refTCB->xStateListItem.prev = (ListXItem_t *)theTCB;
                        break;
                    }
                }
            }while(1);
        }
    }
    return theTCB;
}


BaseTCB_t * pushRun2TermListTCB(BaseTCB_t * toList)
{

    if(toList == pxRun2TermTCBList_Head)
    {
        return toList;
        while(1);
    }

    if(toList != NULL)
    {
        toList->xStateListItem.prev = (ListXItem_t *)pxRun2TermTCBList_Head;
        toList->xStateListItem.next = NULL;
        if(pxRun2TermTCBList_Head != NULL)
        {   // for Run2TermTCBList it is not necessary but just for consistent
            //  to Run2TermTCBList  and debug
            pxRun2TermTCBList_Head->xStateListItem.next = (ListXItem_t *)toList;
        }
        pxRun2TermTCBList_Head = toList;
    }

    return toList;
}

BaseTCB_t * popRun2TermListTCB()
{
    BaseTCB_t * theTCB = pxRun2TermTCBList_Head;

    if(theTCB != NULL)
    {
        pxRun2TermTCBList_Head = (BaseTCB_t *)theTCB->xStateListItem.prev;
        if(NULL != pxRun2TermTCBList_Head)
        {
            pxRun2TermTCBList_Head->xStateListItem.next =NULL;
        }
    }

    return theTCB;
}

BaseTCB_t * getCurrentTCB()
{
    return  pxCurrentTCB;
}
