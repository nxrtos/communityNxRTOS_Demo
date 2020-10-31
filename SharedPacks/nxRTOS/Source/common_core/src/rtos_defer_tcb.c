/* rtos_defer_tcb.c
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
 * DeferTCB is similar to DeferJCB in the management , but the all items
 * for deferring Management is build into BaseTCB_t structure rather than
 * DeferJCB through seperated SoftTimer_ControlBlock binding with JCB.
 *
 * There are pxOsCurrentDeferTCBWaitingList and
 * pxOsOverflowDeferTCBWaitingList to manage the TCB from ready List
 * to WaitingList.
 *
 * As the rule in Run2TermThread_context it is not allowed to invoke
 * rtosDelay(ticks).  Only in Run2BlckThread_context allowed to call
 * rtosDelay(ticks).
 *---------------------------------------------------------------------------*/
#include  "nxRTOSConfig.h"

/// {{{
#if     (RTOS_SUPPORT_DELAY_IN_THREAD == 1)
#include  "rtos_defer_tcb.h"
#include  "rtos_tick_process.h"
#include  "arch4rtos_criticallevel.h"


volatile R2BTCB_t * pxDeferTCBCurrentWaitingList = NULL;
volatile R2BTCB_t * pxDeferTCBOverflowWaitingList = NULL;


R2BTCB_t *  pickFromDeferTCBWaitingList(R2BTCB_t *  theR2BTCB )
{
    BaseTCB_t  * theTCB = (BaseTCB_t  *)theR2BTCB;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    if((theR2BTCB != NULL)) // && (theR2BTCB->xResumeOnTicks != 0))
    {
        BaseTCB_t  * refTCB;
        // {{{  critical section enter  {{{
        arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

        theR2BTCB->xResumeOnTicks = 0;
        refTCB = (BaseTCB_t  *)theTCB->xStateListItem.next;
        if(refTCB != NULL)
        {
            refTCB->xStateListItem.prev = theTCB->xStateListItem.prev;
        }
        refTCB = (BaseTCB_t  *)theTCB->xStateListItem.prev;
        if(refTCB != NULL)
        {
            refTCB->xStateListItem.next = theTCB->xStateListItem.next;
        }
        //  theR2BTCB looks the HEAD of List
        if(pxDeferTCBCurrentWaitingList == theR2BTCB)
        {
            pxDeferTCBCurrentWaitingList = (R2BTCB_t *)theTCB->xStateListItem.next;
        }
        else if(pxDeferTCBOverflowWaitingList == theR2BTCB)
        {
            pxDeferTCBOverflowWaitingList = (R2BTCB_t *)theTCB->xStateListItem.next;
        }

        {// now clean theTCB->xStateListItem;  May not needed
            theTCB->xStateListItem.next = NULL;
            theTCB->xStateListItem.prev = NULL;
        }
        // }}}  critical section exit   }}}
        arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    }
    else
    {   // catch error
        while(1);
    }
    return (R2BTCB_t *)theTCB;
}

R2BTCB_t *  addToDeferTCBWaitingList(R2BTCB_t *  theR2BTCB )
{
    BaseTCB_t  * theTCB = (BaseTCB_t  *)theR2BTCB;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    if((theR2BTCB != NULL) && (theR2BTCB->xResumeOnTicks != 0))
    {
        TickType_t  currentTick;
        // {{{  critical section enter  {{{
        arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
        currentTick =rtosGetRtosTick();
        theR2BTCB->xResumeOnTicks += currentTick;
        if(theR2BTCB->xResumeOnTicks > currentTick)
        {   //  add theSoftTimer into pxOsCurrentSoftTimerWaitingList
            if(pxDeferTCBCurrentWaitingList == NULL)
            {
                pxDeferTCBCurrentWaitingList = theR2BTCB;
                theTCB->xStateListItem.prev = NULL;
                theTCB->xStateListItem.next = NULL;
            }
            else
            {
                R2BTCB_t * refTCB = (R2BTCB_t *)pxDeferTCBCurrentWaitingList;

                while(refTCB->xResumeOnTicks <= theR2BTCB->xResumeOnTicks)
                {   // try to find right refTCB
                    if( ((BaseTCB_t *)refTCB)->xStateListItem.next != NULL)
                    {   //
                        refTCB = (R2BTCB_t *)
                                    ((BaseTCB_t *)refTCB)->xStateListItem.next;
                    }
                    else
                    {   // refSoftTimer  is the last one in SoftTimerWaitingList;
                        break; // out of while
                    }
                }

                if(refTCB->xResumeOnTicks <= theR2BTCB->xResumeOnTicks)
                {   // add theR2BTCB  after refTCB
                    ((BaseTCB_t *)theR2BTCB)->xStateListItem.prev = (ListXItem_t *)refTCB;
                    ((BaseTCB_t *)theR2BTCB)->xStateListItem.next = // NULL
                                ((BaseTCB_t *)refTCB)->xStateListItem.next;
                    ((BaseTCB_t *)refTCB)->xStateListItem.next = (ListXItem_t *)theR2BTCB;
                    if(((BaseTCB_t *)theR2BTCB)->xStateListItem.next != NULL)
                    {
                        refTCB = (R2BTCB_t *)((BaseTCB_t *)theR2BTCB)->xStateListItem.next;
                        ((BaseTCB_t *)refTCB)->xStateListItem.prev = (ListXItem_t *)theR2BTCB;
                    }
                }
                else
                {   // add theR2BTCB  before refTCB
                    ((BaseTCB_t *)theR2BTCB)->xStateListItem.prev =
                                    ((BaseTCB_t *)refTCB)->xStateListItem.prev;
                    ((BaseTCB_t *)theR2BTCB)->xStateListItem.next = (ListXItem_t *)refTCB;
                    ((BaseTCB_t *)refTCB)->xStateListItem.prev = (ListXItem_t *)theR2BTCB;
                    if(refTCB == pxDeferTCBCurrentWaitingList)
                    {   // if the refTCB is the HEAD, then update the HEAD
                        pxDeferTCBCurrentWaitingList = theR2BTCB;
                    }
                    if(((BaseTCB_t *)theR2BTCB)->xStateListItem.prev != NULL)
                    {
                        refTCB = (R2BTCB_t *)((BaseTCB_t *)theR2BTCB)->xStateListItem.prev;
                        ((BaseTCB_t *)refTCB)->xStateListItem.next = (ListXItem_t *)theR2BTCB;
                    }
                }
            }
        }
        else
        {   //  add theR2BTCB into pxDeferTCBOverflowWaitingList
            if(pxDeferTCBOverflowWaitingList == NULL)
            {
                pxDeferTCBOverflowWaitingList = theR2BTCB;
                theTCB->xStateListItem.prev = NULL;
                theTCB->xStateListItem.next = NULL;
            }
            else
            {
                R2BTCB_t * refTCB = (R2BTCB_t *)pxDeferTCBOverflowWaitingList;

                while(refTCB->xResumeOnTicks < theR2BTCB->xResumeOnTicks)
                {
                    if( ((BaseTCB_t *)refTCB)->xStateListItem.next != NULL)
                    {   //
                        refTCB = (R2BTCB_t *)
                                    ((BaseTCB_t *)refTCB)->xStateListItem.next;
                    }
                    else
                    {   // refSoftTimer  is the last one in SoftTimerWaitingList;
                        break; // out of while
                    }
                }

                if(refTCB->xResumeOnTicks < theR2BTCB->xResumeOnTicks)
                {   // add theR2BTCB  after refTCB
                    ((BaseTCB_t *)theR2BTCB)->xStateListItem.prev = (ListXItem_t *)refTCB;
                    ((BaseTCB_t *)theR2BTCB)->xStateListItem.next = // NULL
                                ((BaseTCB_t *)refTCB)->xStateListItem.next;
                    ((BaseTCB_t *)refTCB)->xStateListItem.next = (ListXItem_t *)theR2BTCB;
                    if(((BaseTCB_t *)theR2BTCB)->xStateListItem.next != NULL)
                    {
                        refTCB = (R2BTCB_t *)((BaseTCB_t *)theR2BTCB)->xStateListItem.next;
                        ((BaseTCB_t *)refTCB)->xStateListItem.prev = (ListXItem_t *)theR2BTCB;
                    }
                }
                else
                {   // add theR2BTCB  before refTCB
                    ((BaseTCB_t *)theR2BTCB)->xStateListItem.prev =
                                    ((BaseTCB_t *)refTCB)->xStateListItem.prev;
                    ((BaseTCB_t *)theR2BTCB)->xStateListItem.next = (ListXItem_t *)refTCB;
                    ((BaseTCB_t *)refTCB)->xStateListItem.prev = (ListXItem_t *)theR2BTCB;
                    if(refTCB == pxDeferTCBOverflowWaitingList)
                    {   // if the refTCB is the HEAD, then update the HEAD
                        pxDeferTCBOverflowWaitingList = theR2BTCB;
                    }
                    if(((BaseTCB_t *)theR2BTCB)->xStateListItem.prev != NULL)
                    {
                        refTCB = (R2BTCB_t *)((BaseTCB_t *)theR2BTCB)->xStateListItem.prev;
                        ((BaseTCB_t *)refTCB)->xStateListItem.next = (ListXItem_t *)theR2BTCB;
                    }
                }
            }
        }
        // }}}  critical section exit   }}}
        arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    }
    else
    {   // catch error
        while(1);
    }
    return  (R2BTCB_t *)theTCB;
}

#include    "list_tcb.h"
void vThreadDelay( const TickType_t xTicksToDelay )
{
    if(xTicksToDelay)
    {
        R2BTCB_t * theTCB;
        SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
        // {{{  critical section enter  {{{
        arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
        // it has to be in a Run2BlckThread_context. add check here

        theTCB = (R2BTCB_t *)pickTCBFromRun2BlckTCBList(pxCurrentTCB);
        if(theTCB != NULL)
        {
            theTCB->xResumeOnTicks = xTicksToDelay;
            theTCB->wFlags = Wait4Ticks;
            addToDeferTCBWaitingList(theTCB);
            // at this point need pxCurrentTCB has been removed from Run2BlckTCBList
           arch4rtosReqSchedulerService();// req kernel to re-schedule;
        }
        // }}}  critical section exit   }}}
        arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    }
    else    // for xTicksToDelay == 0, do yield
    {

    }
    return;
}
#endif  /// (RTOS_SUPPORT_DELAY_IN_THREAD == 1) }}}
