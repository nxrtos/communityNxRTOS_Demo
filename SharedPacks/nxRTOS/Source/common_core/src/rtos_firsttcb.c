/* arch4rtos_firsttcb.c
 *
 * nxRTOS Kernel V0.0. 1
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
#include "rtos_start_kernel.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_commit_job.h"
#include  "rtos_new_tcb.h"

// refer to TCB_t * pickTCB(TCB_t * currentTCB )
BaseTCB_t *	firstTCB(JCB_t * theJCB)
{
    BaseTCB_t * theTCB;
    SysCriticalLevel_t  origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{ raise Sys_Critical_Level  to designed Kernel_Critical_Level {{{
   // arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    arch4rtos_iRaiseSysCriticalLevel(0);

    {   // sanity check may put here for debugging.
        if(pxCurrentTCB != NULL)
            while(1);
        if(pxRun2TermTCBList_Head != NULL)
            while(1);
        if(pxRun2BlckTCBList_Head != NULL)
            while(1);
        // more check added after

    }

    // load_job_to_thread(pxReadyListJCB);
    theTCB = createTCB(theJCB);
    if(theTCB == NULL)
    {
        while(1);
    }
    // update pxRun2BlckTCBList_Head or pxRun2TermTCBList_Head
    if(theJCB->pThreadStack ==ThreadStackTempStacking)
    {
        // pxRun2TermTCBList_Head = theTCB;
        pushRun2TermListTCB(theTCB);
    }
    else // if(theJCB->pThreadStack == StackKept)
    {
        // pxRun2BlckTCBList_Head = theTCB;
        addTCBToRun2BlckTCBList(theTCB);
     }

    // update pxCurrentTCB here.
    pxCurrentTCB = theTCB;
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}restore Sys_Critical_Level to original }}}
    return theTCB;
}
