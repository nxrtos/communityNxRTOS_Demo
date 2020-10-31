/* defer_jcb.c
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
 * DeferJCB will be managed through SoftTimerWaitingList
 *---------------------------------------------------------------------------*/

#include  "defer_jcb.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_tick_process.h"
#include  "nxRTOSConfig.h"

#include  "stddef.h"

//volatile
SoftTimer_t * pxOsCurrentSoftTimerWaitingList = NULL;
//volatile
SoftTimer_t * pxOsOverflowSoftTimerWaitingList = NULL;

//debug
int     numberInWaitList = 0;

SoftTimer_t * pickFromSoftTimerWaitingList(SoftTimer_t * theSoftTimer )
{
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    SoftTimer_t *   refSoftTimer;

    // {{{  critical section enter  {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

    numberInWaitList--;
    if((theSoftTimer != NULL)
       && ((theSoftTimer->tmState == Run_State)
           || (theSoftTimer->tmState == Run_4_Job)
           || (theSoftTimer->tmState == Run_4_Thread)
          )                                             )
    {
        // get refSoftTimer the prev of theSoftTimer
        refSoftTimer = (SoftTimer_t *) theSoftTimer->baseXList.prev;
        if(refSoftTimer != NULL)
        {
            refSoftTimer->baseXList.next = theSoftTimer->baseXList.next;
        }

        // get refSoftTimer the next of theSoftTimer
        refSoftTimer = (SoftTimer_t *) theSoftTimer->baseXList.next;
        if(refSoftTimer != NULL)
        {
            refSoftTimer->baseXList.prev = theSoftTimer->baseXList.prev;
        }

        if(pxOsCurrentSoftTimerWaitingList == theSoftTimer)
        {
            pxOsCurrentSoftTimerWaitingList = refSoftTimer;
        }

        if(pxOsOverflowSoftTimerWaitingList == theSoftTimer)
        {
            pxOsOverflowSoftTimerWaitingList = refSoftTimer;
        }
        theSoftTimer->tmState = Stop_State;
        // extra clean
        theSoftTimer->baseXList.prev = NULL;
        theSoftTimer->baseXList.next = NULL;
    }
    else
    {   // catch error
        while(1);
    }
    // }}}  critical section exit   }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);

    // catch error
    if((pxOsCurrentSoftTimerWaitingList != NULL) &&
       ((pxOsCurrentSoftTimerWaitingList->tmState != Run_State) &&
        (pxOsCurrentSoftTimerWaitingList->tmState != Run_4_Job) &&
        (pxOsOverflowSoftTimerWaitingList->tmState != Run_4_Thread)
       )                                                          )

    {
        while(1);
    }

    if((pxOsOverflowSoftTimerWaitingList != NULL) &&
       ((pxOsOverflowSoftTimerWaitingList->tmState != Run_State) &&
        (pxOsOverflowSoftTimerWaitingList->tmState != Run_4_Job) &&
        (pxOsOverflowSoftTimerWaitingList->tmState != Run_4_Thread)
            )                                                          )
    {
        while(1);
    }

    return theSoftTimer;
}

// add SoftTimer for deferred Job to WaitingList
SoftTimer_t * addToSoftTimerWaitingList(SoftTimer_t * theSoftTimer )
{
    TickType_t currentTick;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{  critical section enter  {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    numberInWaitList++;
    if((theSoftTimer != NULL)
            //&& (theSoftTimer->tmState == Stop_State)
            && (theSoftTimer->pActionObj != NULL)
                                                        )
    {
        currentTick = rtosGetRtosTick();
        theSoftTimer->xActionOnTicks = currentTick
                                       + theSoftTimer->xTimerPeriodInTicks;
        if(theSoftTimer->xActionOnTicks >= currentTick)
        {   //  add theSoftTimer into pxOsCurrentSoftTimerWaitingList
            if(pxOsCurrentSoftTimerWaitingList == NULL)
            {
                pxOsCurrentSoftTimerWaitingList = theSoftTimer;
                theSoftTimer->baseXList.prev = NULL;
                theSoftTimer->baseXList.next = NULL;
            }
            else
            {
                SoftTimer_t * refSoftTimer =
                                (SoftTimer_t *)pxOsCurrentSoftTimerWaitingList;

                while(refSoftTimer->xActionOnTicks < theSoftTimer->xActionOnTicks)
                {
                    if( refSoftTimer->baseXList.next != NULL)
                    {   //
                        refSoftTimer = (SoftTimer_t *)
                                                refSoftTimer->baseXList.next;
                    }
                    else
                    {   // refSoftTimer  is the last one in SoftTimerWaitingList;
                        break; // out of while
                    }
                }

                if(refSoftTimer->xActionOnTicks < theSoftTimer->xActionOnTicks)
                {   // add theSoftTimer after refSoftTimer
                    theSoftTimer->baseXList.prev = (SoftTimer_t *)refSoftTimer;
                    theSoftTimer->baseXList.next = refSoftTimer->baseXList.next;
                    refSoftTimer->baseXList.next = (SoftTimer_t *)theSoftTimer;
                }
                else
                {   // add theSoftTimer before refSoftTimer
                    theSoftTimer->baseXList.prev = refSoftTimer->baseXList.prev;
                    theSoftTimer->baseXList.next = (SoftTimer_t *)refSoftTimer;
                    refSoftTimer->baseXList.prev = (SoftTimer_t *)theSoftTimer;
                    if(pxOsCurrentSoftTimerWaitingList == refSoftTimer)
                    {   //  if refSoftTimer is current
                        //  make theSoftTimer to be new HEAD
                        pxOsCurrentSoftTimerWaitingList = theSoftTimer;
                    }
                    else
                    { // if refSoftTimer is not the HEAD, update the prev's next
                        refSoftTimer = theSoftTimer->baseXList.prev;
                        if(refSoftTimer != NULL)
                        {
                            refSoftTimer->baseXList.next =
                                                 (SoftTimer_t *)theSoftTimer;
                        }
                    }
                }
            }
            // end of add theSoftTimer into pxOsCurrentSoftTimerWaitingList
        }
        else
        {   //  add theSoftTimer into pxOsOverflowSoftTimerWaitingList
            if(pxOsOverflowSoftTimerWaitingList == NULL)
            {
                pxOsOverflowSoftTimerWaitingList = theSoftTimer;
                theSoftTimer->baseXList.prev = NULL;
                theSoftTimer->baseXList.next = NULL;
            }
            else
            {
                SoftTimer_t *   refSoftTimer =
                                              pxOsOverflowSoftTimerWaitingList;
                while(refSoftTimer->xActionOnTicks <= theSoftTimer->xActionOnTicks)
                {
                    if( refSoftTimer->baseXList.next != NULL)
                    {   //
                        refSoftTimer = (SoftTimer_t *)
                                                refSoftTimer->baseXList.next;
                    }
                    else
                    {   // refSoftTimer  is the last one in SoftTimerWaitingList;
                        break; // out of while
                    }
                }

                if(refSoftTimer->xActionOnTicks <= theSoftTimer->xActionOnTicks)
                {   // add theSoftTimer after refSoftTimer
                    theSoftTimer->baseXList.prev = (SoftTimer_t *)refSoftTimer;
                    theSoftTimer->baseXList.next = refSoftTimer->baseXList.next;
                    refSoftTimer->baseXList.next = (SoftTimer_t *)theSoftTimer;
                }
                else
                {   // add theSoftTimer before refSoftTimer
                    theSoftTimer->baseXList.prev = refSoftTimer->baseXList.prev;
                    theSoftTimer->baseXList.next = (SoftTimer_t *)refSoftTimer;
                    refSoftTimer->baseXList.prev = (SoftTimer_t *)theSoftTimer;
                    if(pxOsOverflowSoftTimerWaitingList == refSoftTimer)
                    {   //  if refSoftTimer is current
                        //  make theSoftTimer to be new HEAD
                        pxOsOverflowSoftTimerWaitingList = theSoftTimer;
                    }
                    else
                    { // if refSoftTimer is not the HEAD, update the prev's next
                        refSoftTimer = theSoftTimer->baseXList.prev;
                        if(refSoftTimer != NULL)
                        {
                            refSoftTimer->baseXList.next =
                                                (SoftTimer_t *)theSoftTimer;
                        }
                    }

                }
            }
            // end of add theSoftTimer into pxOsOverflowSoftTimerWaitingList
        }
        theSoftTimer->tmState = Run_4_Job;//Run_State;
    }
    else
    {   // catch error
        while(1);
    }

    {   //  catch error
        int  countInWaitList = 0;
        SoftTimer_t * checkTimer;

        checkTimer = (SoftTimer_t *)pxOsOverflowSoftTimerWaitingList;
        while(checkTimer != NULL)
        {
            countInWaitList++;
            checkTimer = checkTimer->baseXList.next;
        }

        checkTimer = (SoftTimer_t *)pxOsCurrentSoftTimerWaitingList;
        while(checkTimer != NULL)
        {
            countInWaitList++;
            checkTimer = checkTimer->baseXList.next;
        }
        // Caught error
        if(countInWaitList != numberInWaitList)
            while(1);
    }
    // }}}  critical section exit   }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);

    // catch error
    if((pxOsCurrentSoftTimerWaitingList != NULL) &&
       ((pxOsCurrentSoftTimerWaitingList->tmState != Run_State) &&
        (pxOsCurrentSoftTimerWaitingList->tmState != Run_4_Job) &&
        (pxOsCurrentSoftTimerWaitingList->tmState != Run_4_Thread)
            )                                                       )
    {
        while(1);
    }

    if((pxOsOverflowSoftTimerWaitingList != NULL) &&
       ((pxOsOverflowSoftTimerWaitingList->tmState != Run_State) &&
        (pxOsOverflowSoftTimerWaitingList->tmState != Run_4_Job) &&
        (pxOsOverflowSoftTimerWaitingList->tmState != Run_4_Thread)
       )                                                            )
    {
        while(1);
    }

    if((theSoftTimer->tmState != Run_State) &&
       (theSoftTimer->tmState != Run_4_Job) &&
       (theSoftTimer->tmState != Run_4_Thread)
                                                  )
    {
        while(1);
    }

    return theSoftTimer;
}
