/* rtos_softtimer.c
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

#include    "rtos_softtimer.h"
#include    "rtos_tick_process.h"
#include    "arch4rtos_criticallevel.h"
#include    "rtos_jcb.h"
#include    "list_jcb.h"
#include    "defer_jcb.h"   /* some of API should move to tickProcess.h */
#include    "nxRTOSConfig.h"


/// {{{ freeSoftTimerList   {{{
#if     (SYS_SOFTTIMER_NUM)
static  SoftTimer_t     freeSoftTimer[SYS_SOFTTIMER_NUM];
#endif

/// }}} freeSoftTimerList   }}}
int     xFreeSoftTimerNum = 0;

SoftTimer_t *   pFreeSoftTimerList = NULL;

int     initFreeSoftTimerList()
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

#if     (SYS_SOFTTIMER_NUM)     //  {{{

    for(int i = 0; i < SYS_SOFTTIMER_NUM -1; i++)
    {   // in freeList only baseXList.next need to initialize
        freeSoftTimer[i].baseXList.next = //(SoftTimer_t *)
                                                &freeSoftTimer[i+1];
        freeSoftTimer[i].tmState = Free_State;
    }
    freeSoftTimer[SYS_SOFTTIMER_NUM -1].baseXList.next = NULL;
    pFreeSoftTimerList = &freeSoftTimer[0];
    xFreeSoftTimerNum = SYS_SOFTTIMER_NUM;
    retV = xFreeSoftTimerNum;
#endif  //  (SYS_SOFTTIMER_NUM) }}}

    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return retV;
}

SoftTimer_t *   xAcquireFreeSoftTimer()
{
    SoftTimer_t *   theSoftTimer;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter  {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

    theSoftTimer = pFreeSoftTimerList;

    if(theSoftTimer != NULL)
    {
        pFreeSoftTimerList = (SoftTimer_t *)(theSoftTimer->baseXList.next);
        xFreeSoftTimerNum--;
    }

    if(theSoftTimer != NULL)
    {
        theSoftTimer->tmState = Stop_State;
        // below pre-setting may not needed
        theSoftTimer->pActionObj = NULL;
        theSoftTimer->actOption = AutoFree;
        theSoftTimer->xTimerPeriodInTicks = 0;
        theSoftTimer->xActionOnTicks = 0;
    }

    // critical section exit    }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return theSoftTimer;
}

int xReleaseSoftTimer(SoftTimer_t * theSoftTimer)
{
    int     theNumber;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    int     debug;

    // sanity check
    if (origCriticalLevel < RTOS_SYSCRITICALLEVEL)
    {
        while(1);
    }

    // {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    // sanity check stop first.
    debug = xFreeSoftTimerNum +1;

    {
        SysCriticalLevel_t checkLevel = arch4rtos_iGetSysCriticalLevel();
        if(checkLevel != RTOS_SYSCRITICALLEVEL)
        {
            while(1);
        }
    }
    if(theSoftTimer != NULL)
    {
        if((Run_State == theSoftTimer->tmState ) ||
           (Run_4_Job == theSoftTimer->tmState) ||
           (Run_4_Thread == theSoftTimer->tmState) ||
                                                    0)
        {  // if it's still running, stop it first
            xStopSoftTimer(theSoftTimer);
            theSoftTimer->tmState = Stop_State;
        }
    }

#if     1
    if(theSoftTimer != NULL)
    {
        if((Run_State == theSoftTimer->tmState) ||
           (Run_4_Job == theSoftTimer->tmState) ||
           (Run_4_Thread == theSoftTimer->tmState) ||
           (Stop_State == theSoftTimer->tmState))
        {   // TODO what happen if the linked JCB is a static ???
            JCB_t * theJCB = (JCB_t *)theSoftTimer->pActionObj;
            if(NULL != theJCB)
            {
                xappendFreeListJCB(theJCB);
            }
        }
        // clean theSoftTimer  up, may not really needed
        theSoftTimer->pActionObj = NULL;
        theSoftTimer->actOption = AutoFree;
        theSoftTimer->xActionOnTicks = 0;
        theSoftTimer->xTimerPeriodInTicks = 0;
    }
#endif


    if(theSoftTimer != NULL)
    {
        //if(theSoftTimer->tmState == Stop_State)
        {
            theSoftTimer->baseXList.next = (SoftTimer_t *)pFreeSoftTimerList;
            pFreeSoftTimerList = theSoftTimer;
            xFreeSoftTimerNum++;
        }
    }
    theNumber = xFreeSoftTimerNum;
    // critical section exit    }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);

    if( debug != xFreeSoftTimerNum)
        while(1);
    return theNumber;
}

/// {{{
#if     0
// this is kind of similar to rtos_commit_deferjob, but from SoftTimer
// perspective and give more SoftTimer settings.
SoftTimer_t *   pxSetupSoftTimer(SoftTimer_t *  theSoftTimer,
                                 SoftTimerActOption_t   theActOption,
                                 TickType_t      xPeriodInTicks,
                                 JCB_t * pTimedJCB,
                                 int iJobPar,
                                 void * pxJobData,
                                 StackType_t    * pThreadStack,
                                 int iStackSize,
                                 JobHandlerEntry    * pJobHandler,
                                 uint32_t xJobPriority,
                                 JCB_ActOption_t autoAct     )
{
    SoftTimer_t * retSoftTimer = theSoftTimer;

    if(retSoftTimer == NULL)
    {   // if the input is not a real pointer to SoftTimer, create one.
        retSoftTimer = xAcquireFreeSoftTimer();
    }

    if(retSoftTimer != NULL)
    {
        JCB_t * theJCB = pTimedJCB;

        if(theJCB == NULL)
        {
            theJCB = xpickFreeListJCB();
        }

        if(theJCB != NULL)
        {
            retSoftTimer->actOption = theActOption;
            retSoftTimer->xTimerPeriodInTicks = xPeriodInTicks;
            retSoftTimer->tmState = Stop_State;
            retSoftTimer->pXItem = (ListXItem_t *) theJCB;
            theJCB->iJobPar = iJobPar ;
            theJCB->pJobData = pxJobData;
//                theJCB->pThread = NULL;
            theJCB->pThreadStack = pThreadStack;
            theJCB->stackSize = iStackSize;
            theJCB->threadEntryFunc = pJobHandler;
            theJCB->uxPriority = xJobPriority;
            theJCB->stateOfJcb = JCB_STATE_COMMIT;
            theJCB->actOption = autoAct;
            theJCB->pxBindingObj = NULL; //theSoftTimer;  /// or NULL ?
#if 0
            /// Immediately start
            if (retSoftTimer != addToSoftTimerWaitingList(retSoftTimer ))
            {   // something wrong here.
                while(1);
            }
#endif

        }
        else
        {   // failed to pickFreeListJCB,
            if((NULL ==  theSoftTimer) && (retSoftTimer != NULL))
            {   // release the acquired SoftTimer
                xReleaseSoftTimer(retSoftTimer);
                retSoftTimer = NULL;
            }
        }
    }
    return retSoftTimer;
}

#endif
/// }}}


SoftTimer_t *   xCreateSoftTimer(SoftTimer_t *          theSoftTimer,
                                 SoftTimerActOption_t   theActOption,
                                 TickType_t             xPeriodInTicks,
                                 JCB_t *                pTimedJCB)
{
    SoftTimer_t * retSoftTimer = theSoftTimer;

    if(retSoftTimer == NULL)
    {   // if the input is not a real pointer to SoftTimer, create one
        // from sysSoftTimerPool
        retSoftTimer = xAcquireFreeSoftTimer();
    }

    if(retSoftTimer != NULL)
    {
        theSoftTimer->pActionObj = pTimedJCB;
        theSoftTimer->actOption = theActOption;
        theSoftTimer->xTimerPeriodInTicks = xPeriodInTicks;
    }

    return retSoftTimer;
}

SoftTimer_t *   xStartSoftTimer(SoftTimer_t * theSoftTimer )
{
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{ critical section enter  {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    // sanity check
    if((theSoftTimer != NULL)
            && (theSoftTimer->tmState != Free_State)
            //&& (theSoftTimer->tmState == Stop_State)
                                                        )
    {
        addToSoftTimerWaitingList(theSoftTimer);

    #if     0
        if((theSoftTimer != NULL) && (theSoftTimer->tmState == Stop_State))
        {   // ignore   invilid SoftTimer and  tmState other than Stop_State,
            // such as      Free_State, Run_State
            theSoftTimer->xActionOnTicks = currentTick
                                              + theSoftTimer->xTimerPeriodInTicks;
            if(theSoftTimer->xActionOnTicks >= currentTick)
            {   // add the SoftTimer to WaitingJCBList
                //  TODO((JCB_t *)(theSoftTimer->pXItem))->xTheListItem.next
            }
            else
            {   // TODO add the SoftTimer to OverflowWaitingJCBList

            }
            theSoftTimer->tmState = Run_State;
        }
    #endif
    }
    // }}}  critical section exit    }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);

    return  theSoftTimer;
}


SoftTimer_t *   xStopSoftTimer(SoftTimer_t * theSoftTimer)
{
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{ critical section enter  {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if((theSoftTimer != NULL) && (theSoftTimer->tmState == Run_State))
    {
        pickFromSoftTimerWaitingList(theSoftTimer);
        theSoftTimer->tmState = Stop_State;
    }
    // }}}  critical section exit    }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return  theSoftTimer;
}


/// {{{ deferJobSoftTimerList, overflowdeferJobSoftTimerList {{{




/// }}} deferJobSoftTimerList  overflowdeferJobSoftTimerList }}}
