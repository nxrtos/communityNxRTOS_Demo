/* rtos_nexttcb.c
 *
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
 * Implementation of functions defined in arch4rtos.h for the architecture of
 * ARM CM4F.
 * Refer to arch4rtos.tx for design consideration.
 *---------------------------------------------------------------------------*/

#include "arch4rtos.h"
#include "arch4rtos_firstjob.h"
#include "list_tcb.h"
#include "list_jcb.h"
#include "rtos_start_kernel.h"
#include  "arch4rtos_criticallevel.h"
#include  "next_tcb.h"
#include  "rtos_commit_job.h"
#include  "rtos_new_tcb.h"
// refer to TCB_t * pickTCB(TCB_t * currentTCB )
// this is for remove currentTCB from either Run2TermListTCB or Run2BlckListTCB
// may implement as an unified function to share with requireSemaphore or
// Mutex to move currentTCB from Run2BlckListTCB to WaitingSemaListTCB
// or WaitingMutexListTCB
BaseTCB_t *	nextTCB(JCB_t * theJCB)
{
    BaseTCB_t * pxSelectToRunTCB;
    BaseTCB_t * theTCB;
    SysCriticalLevel_t  origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{ raise Sys_Critical_Level  to designed Kernel_Critical_Level {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    //arch4rtos_iRaiseSysCriticalLevel(0);
    theTCB = pxCurrentTCB;
#ifdef  RTOS_THREAD_STACK_OVERFLOW_TRACE
    if(theTCB != NULL)
    {
        if(*(theTCB->pxStackMarkPosition) != RTOS_STACK_OVERFLOW_TRACE_DEFAULTMARK)
        {   // failed on Mark check, stack has overflow
            while(1);
        }
    }
#endif
    // remove it from runningList
    if(pxCurrentTCB == pxRun2TermTCBList_Head)
    {   // if pxCurrentTCB is a Run2TermTCB, it must be the Head
        popRun2TermListTCB();
        //pxRun2TermTCBList_Head = (TCB_t *) pxRun2TermTCBList_Head->xStateListItem.prev;
    }
    else //    if(pxCurrentTCB == pxRun2BlckTCBList_Head)
    {  // otherwise pxCurrentTCB must be Run2BlckTCB
        pickTCBFromRun2BlckTCBList(pxCurrentTCB);
        //pxRun2BlckTCBList_Head = (TCB_t *) pxRun2BlckTCBList_Head->xStateListItem.prev;
    }

    // now look through pxRun2TermTCBList_Head , pxRun2BlckTCBList_Head and readyJCB
    // below will be similar to Scheduler_ pickTCB.
    // find the highest priority in readyJob and runningThread.
    // the highest priority of runningThread can be find from pxSelectToRunTCB.
    // it can could be NULL if pxCurrentTCB is going to terminating itself and
    // it is the highest priority of runningThread.
    // otherwise, it should be either same as pxRun2TermTCBList_Head or
    //  pxRun2BlckTCBList_Head.

    // so first re-elect pxSelectToRunTCB among pxRun2TermTCBList_Head
    // pxRun2BlckTCBList_Head
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

    // at this point pxSelectToRunTCB has elected among pxRun2TermTCBList_Head
    // pxRun2BlckTCBList_Head.  Now need to check with pxReadyListJCB
    if(pxReadyListJCB == NULL)
    {   // pxSelectToRunTCB has not to be NULL, or SysIdleJob take over.
        if(pxSelectToRunTCB == NULL)
        {
            //while(1); // replace by SysIdleJob
            pxSelectToRunTCB = pxCurrentTCB;
        }
    }
    else if((pxSelectToRunTCB == NULL) || (pxReadyListJCB->uxPriority < pxSelectToRunTCB->uxPriority))
    {   // readyJCB has higher priority, so load the Job and replace pxSelectToRunTCB
        JCB_t * theJCB;

        theJCB = pickReadyListJCB(pxReadyListJCB); // the pick will update pxReadyListJCB
        // load_job_to_thread(pxReadyListJCB);
        pxSelectToRunTCB = createTCB(theJCB);

        if(pxSelectToRunTCB == NULL)
        {   // failed to createTCB, more likely because of no enough space
            // for the StackSpace , put back the theJCB to ReadyList
            addReadyListJCB(theJCB);
            if(theJCB->pThreadStack == ThreadStackTempStacking)
            {   // the readyJCB is StackShared, promote
                if( pxRun2TermTCBList_Head == NULL)
                {   // theJCB has require stack_space too big to satisfy
                    while(1);
                }
                else
                {
                    if(pxRun2TermTCBList_Head->uxPriority > theJCB->uxPriority)
                    {   // promote the
                        pxRun2TermTCBList_Head->uxPriority = theJCB->uxPriority;
                        //pxRun2TermTCBList_Head->uxRtosPromotePriority = theJCB->uxPriority;
                    }
                    else
                    {   // seems something wrong
                        while(1);
                    }
                }
            }
        }
        else
        {
            // update pxRun2BlckTCBList_Head or pxRun2TermTCBList_Head
            //  and pxSelectToRunTCB
            if(theJCB->pThreadStack ==ThreadStackTempStacking)
            {
                pxSelectToRunTCB->xStateListItem.prev = (ListXItem_t *) pxRun2TermTCBList_Head;
                pxRun2TermTCBList_Head = pxSelectToRunTCB;
            }
            else // if(theJCB->pThreadStack == StackKept)
            {
                pxSelectToRunTCB->xStateListItem.prev = (ListXItem_t *) pxRun2BlckTCBList_Head;
                pxRun2BlckTCBList_Head = pxSelectToRunTCB;
            }
        }
    }

    {   // in general at this moment should see theTCB == pxCurrentTCB
        //  and going to update pxCurrentTCB to new value. But the assumption
        // may change in the future if needed.
        UBaseType_t  xReadyJobPriority = RTOS_TERMINATION_THREAD_PRIORITY;
        UBaseType_t  xToRunThreadPriority = RTOS_TERMINATION_THREAD_PRIORITY;

        if (pxSelectToRunTCB != NULL)
        {
            xToRunThreadPriority = pxSelectToRunTCB->uxPriority;
        }

        if (pxReadyListJCB != NULL)
        {
            xReadyJobPriority = pxReadyListJCB->uxPriority;
        }

        if(xReadyJobPriority < xToRunThreadPriority)
        {
            // pick the JCB and prepare TCB and Thread_stack for it

        }
        else
        {
            if(xToRunThreadPriority != RTOS_TERMINATION_THREAD_PRIORITY)
            {   // pick theTCB  from RunningThread
                theTCB = pxSelectToRunTCB;
            }
            else
            {  // in real case can't be here, as it means both readyJobList and
               // toRunTCB are empty. seems system going shutdown.
               // or define a SysIdle_Job to handle this situation.

            }
        }

    }

    // update pxCurrentTCB here.
    pxCurrentTCB = theTCB;

#ifdef  RTOS_THREAD_STACK_OVERFLOW_TRACE
    if(NULL != pxRun2TermTCBList_Head)
    {  // to resumed Shotlive_Thtread
    	if(pxRun2TermTCBList_Head->pxStackMarkPosition)
    	{  // restore the StackBorderMark
    		*(pxRun2TermTCBList_Head->pxStackMarkPosition) =
                            RTOS_STACK_OVERFLOW_TRACE_DEFAULTMARK;
    	}

    }
#endif

    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}restore Sys_Critical_Level to original }}}
    return theTCB;
}
