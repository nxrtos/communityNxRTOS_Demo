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
 * at the moment in the RTOS system, update it to pxCurrentTCB.
 * it look between readyJobList, readyThreadList to find the
 * highest priority of Thread to serve to.
 * for performance concern,  a 2-wayLinkList for each prioritizedReadyJobList
 * and prioritizedRunningThreadList.
 *  There is a runningToTermiThreadList to kept all runningToTermiThread
 *  in List and ordered by preemption and stacked.
 *---------------------------------------------------------------------------*/

#include    "pick_tcb.h"
#include    "list_tcb.h"
#include    "list_jcb.h"
#include    "arch4rtos_criticallevel.h"
#include    "rtos_new_tcb.h"
#include    "nxRTOSConfig.h"

BaseTCB_t * pickTCB(BaseTCB_t * thisTCB )
{
    BaseTCB_t * pxSelectToRunTCB;
	SysCriticalLevel_t  origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

	// {{{ raise Sys_Critical_Level  to designed Kernel_Critical_Level {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

	if(pxCurrentTCB == NULL)
	{
		pxCurrentTCB = thisTCB;
	}

#if     1
	// extra check, could be omitted
    if(thisTCB != pxCurrentTCB)
    {
        while(1);
    }
#endif

#ifdef  RTOS_THREAD_STACK_OVERFLOW_TRACE
    // this the point may switch out from  pxCurrentTCB
    if(pxCurrentTCB != NULL)
    {   /// chek the mark
        if(*(pxCurrentTCB->pxStackMarkPosition) != RTOS_STACK_OVERFLOW_TRACE_DEFAULTMARK)
        {   // failed on Mark check, stack has overflow
            while(1);
        }
    }
#endif

    // find the highest priority in readyJob and runningThread.
    // the highest priority of runningThread can be find from pxSelectToRunTCB.
    // it can could be NULL if pxCurrentTCB is going to terminating itself and
    // it is the highest priority of runningThread.
    // otherwise, it should be either same as pxRun2TermTCBList_Head or
    //  pxRun2BlckTCBList_Head.

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
        if(theJCB == NULL)
        {   // this should never happen as pxReadyListJCB != NULL. may remove
            while(1);
        }
        // load_job_to_thread(pxReadyListJCB);
        pxSelectToRunTCB = createTCB(theJCB);
        // update pxRun2BlckTCBList_Head or pxRun2TermTCBList_Head
        //  and pxSelectToRunTCB
        if(pxSelectToRunTCB == NULL)
        {   // failed to acquire TCB or StackSpace
            // push theJCB back to ReadyListJCB
            addReadyListJCB(theJCB);

            // promote Running TCBList_Head to
            if(theJCB->pThreadStack == ThreadStackTempStacking)
            {   // promote currentRun2TermTCB
                if((pxRun2TermTCBList_Head != NULL) &&
                        (pxRun2TermTCBList_Head->uxPriority >= theJCB->uxPriority))
                {   //
                    pxRun2TermTCBList_Head->uxPriority = theJCB->uxPriority;
                }
                else
                {   // looks something wrong. it may not be fatal but debug
                    while(1);
                }
            }
            else // if(theJCB->pThreadStack == StackKept)
            {   // for stack NOT Shared, it seems no solution but just do the
                //  similar to promote current executing in hope to terminate
                if((pxRun2BlckTCBList_Head != NULL) &&
                        (pxRun2BlckTCBList_Head->uxPriority > theJCB->uxPriority))
                {   //
                    pxRun2BlckTCBList_Head->uxPriority = theJCB->uxPriority;
                }
                else
                {   // looks something wrong. it may not be fatal but debug
                    while(1);
                }
             }
        }
        else
        {
            if(theJCB->pThreadStack ==ThreadStackTempStacking)
            {
                pushRun2TermListTCB(pxSelectToRunTCB);
            }
            else // if(theJCB->pThreadStack == StackKept)
            {
                addTCBToRun2BlckTCBList(pxSelectToRunTCB);
             }
        }
    }

#ifdef  RTOS_THREAD_STACK_OVERFLOW_TRACE
    // this the point will switch in into  pxCurrentTCB
    if(pxSelectToRunTCB != NULL)
    {   /// restore  the mark
        *(pxSelectToRunTCB->pxStackMarkPosition) = RTOS_STACK_OVERFLOW_TRACE_DEFAULTMARK;
    }
#endif

    if(pxSelectToRunTCB != NULL)
    {   // update pxCurrentTCB here.
        pxCurrentTCB = pxSelectToRunTCB;
    }
    else
    {  // prepare for value to return
       pxSelectToRunTCB = pxCurrentTCB;
    }

    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}restore Sys_Critical_Level to original }}}
	return pxSelectToRunTCB;
}
