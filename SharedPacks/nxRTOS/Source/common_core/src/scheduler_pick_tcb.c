/* scheduler_pick_tcb.c
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
 * pickTCB() will try to find and return the Thread with highest priority
 * at the moment in the RTOS system, update it to getCurrentTCB().
 * it look between readyJobList, readyThreadList to find the
 * highest priority of Thread to serve to.
 * for performance concern,  a 2-wayLinkList for each prioritizedReadyJobList
 * and prioritizedRunningThreadList.
 *  There is a runningToTermiThreadList to kept all runningToTermiThread
 *  in List and ordered by preemption and stacked.
 *---------------------------------------------------------------------------*/

#include  "pick_tcb.h"
#include  "rtos_jcb_ready_list.h"
#include  "rtos_tcb_live_list.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_create_tcb.h"
#include  "rtos_sema_base.h"
#include  "rtos_semaphore.h"
#include  "nxRTOSConfig.h"

LiveTCB_t * pickTCB(LiveTCB_t * thisTCB )
{
    LiveTCB_t * pxSelectToRunTCB;
	SysCriticalLevel_t  origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

	// {{{ raise Sys_Critical_Level  to designed Kernel_Critical_Level {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

	if(getCurrentTCB() == NULL)
	{
		//getCurrentTCB() = thisTCB;
		updateCurrentTCB();
	}

#if     1
	// extra check, could be omitted
    if(thisTCB != getCurrentTCB())
    {
        while(1);
    }
#endif

#ifdef  RTOS_THREAD_STACK_OVERFLOW_TRACE
    // this the point may switch out from  getCurrentTCB()
    if(getCurrentTCB() != NULL)
    {   /// chek the mark
        if(*(getCurrentTCB()->pxStackMarkPosition) != RTOS_STACK_OVERFLOW_TRACE_DEFAULTMARK)
        {   // failed on Mark check, stack has overflow
            while(1);
        }
    }
#endif

    // find the highest priority in readyJob and runningThread.
    // the highest priority of runningThread can be find from pxSelectToRunTCB.
    // it can could be NULL if getCurrentTCB() is going to terminating itself and
    // it is the highest priority of runningThread.
    // otherwise, it should be either same as getCurrentRun2TermTCB() or
    //  getCurrentRun2BlkTCB().

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

    // at this point pxSelectToRunTCB has elected among getCurrentRun2TermTCB()
    // getCurrentRun2BlkTCB().  Now need to check with getReadyJCB()
    if(getReadyJCB() == NULL)
    {   // pxSelectToRunTCB has not to be NULL, or SysIdleJob take over.
        if(pxSelectToRunTCB == NULL)
        {
            //while(1); // replace by SysIdleJob
            pxSelectToRunTCB = getCurrentTCB();
        }
    }
    else if((pxSelectToRunTCB == NULL) || (getReadyJCB()->uxPriority < pxSelectToRunTCB->uxPriority))
    {   // readyJCB has higher priority, so load the Job and replace pxSelectToRunTCB
        JCB_t * theJCB;

        theJCB = pxRemoveFromReadyJCBList(getReadyJCB()); // the pick will update getReadyJCB()
        if(theJCB == NULL)
        {   // this should never happen as getReadyJCB() != NULL. may remove
            while(1);
        }
        // load_job_to_thread(getReadyJCB());
        pxSelectToRunTCB = rtos_create_tcb(theJCB);
        // update getCurrentRun2BlkTCB() or getCurrentRun2TermTCB()
        //  and pxSelectToRunTCB
        if(pxSelectToRunTCB == NULL)
        {   // failed to acquire TCB or StackSpace
            // push theJCB back to ReadyListJCB
            pxInsertToReadyJCBList(theJCB);

            // promote Running TCBList_Head to
            if(theJCB->pThreadStack == ThreadStackTempStacking)
            {   // promote currentRun2TermTCB
                if((getCurrentRun2TermTCB() != NULL) &&
                        (getCurrentRun2TermTCB()->uxPriority >= theJCB->uxPriority))
                {   //
                    getCurrentRun2TermTCB()->uxPriority = theJCB->uxPriority;
                }
                else
                {   // looks something wrong. it may not be fatal but debug
                    while(1);
                }
            }
          else // if(theJCB->pThreadStack == StackKept)
          {   // for stack NOT Shared, it seems no solution but just do the
            //  similar to promote current executing in hope to terminate
            if((getCurrentRun2BlkTCB() != NULL) &&
                    (getCurrentRun2BlkTCB()->uxPriority > theJCB->uxPriority))
            {   //
                getCurrentRun2BlkTCB()->uxPriority = theJCB->uxPriority;
            }
            else
            {   // looks something wrong. it may not be fatal but debug
                while(1);
            }
          }
        }
        else
        { // successful create stack space to load JCB to execution
          if(theJCB->pThreadStack ==ThreadStackTempStacking)
          {
            pushRun2TermListTCB(pxSelectToRunTCB);
          }
          else // if(theJCB->pThreadStack == StackKept)
          {
            insertTCBToRun2BlckTCBList(pxSelectToRunTCB);
          }
        }
    }

#ifdef  RTOS_THREAD_STACK_OVERFLOW_TRACE
    // this the point will switch in into  getCurrentTCB()
    if(pxSelectToRunTCB != NULL)
    {   /// restore  the mark
        *(pxSelectToRunTCB->pxStackMarkPosition) = RTOS_STACK_OVERFLOW_TRACE_DEFAULTMARK;
    }
#endif

    if(pxSelectToRunTCB != NULL)
    {   // update getCurrentTCB() here.
        //getCurrentTCB() = pxSelectToRunTCB;
        updateCurrentTCB();
    }
    else
    {  // prepare for value to return
       pxSelectToRunTCB = getCurrentTCB();
    }

    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}restore Sys_Critical_Level to original }}}
	return pxSelectToRunTCB;
}
