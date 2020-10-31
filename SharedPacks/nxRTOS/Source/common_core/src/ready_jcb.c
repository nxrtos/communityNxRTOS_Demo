/* ready_jcb.c
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
 * Implementation of functions for job control
 *---------------------------------------------------------------------------*/


#include  "list_jcb.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_tick_process.h"
#include  "nxRTOSConfig.h"


/// {{{ readyListJCB {{{
// readyListJCB holding the list for ready JCBs in priority order
// readyListJCB_Head point to the highest priority of ready JCB
// and readyListJCB_Tail to the lowest priority of ready JCB
//
// pickReadyListJCB will take the ready JCB from readyListJCB_Head
// and addReadyListJCB will insert the ready JCB into readyListJCB according
// the priority order.
//
// readyListJCB will be double link in order to remove a JCB from readyListJCB
// when require. (aka. cancel a Job).
// pickReadyListJCB will be a special case of removeReadyListJCB

JCB_t * pxReadyListJCB = NULL;
JCB_t * pxReadyListJCB_Tail = NULL;  // will be removed

static	JCB_t * pReadyListJCB_Head[RTOS_THREAD_PRIORITY_NUM];
static	JCB_t * pReadyListJCB_Tail[RTOS_THREAD_PRIORITY_NUM];

void initReadyListJCB()
{
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

     // critical section enter
     arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
	for(int i = 0; i < RTOS_THREAD_PRIORITY_NUM; i++)
	{
		pReadyListJCB_Head[i] = NULL;
		pReadyListJCB_Tail[i] = NULL;
	}
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
}

// add a JCB into ReadyList.  This happens on either commit a immediate Job
// or a conditional Job find the condition satisfied.
// condition could be as
//      Deferred Job reached the action tick-time, or
//      Mutexed Job / or Semaphored Job acquired the Mutex/Semaphore.
JCB_t * addReadyListJCB(JCB_t * toReadyList)
{
    JCB_t * theJCB;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
	theJCB = pxReadyListJCB;
	do{
		if(theJCB == NULL)
		{	// reached the END of ReadyListJCB, add after readyListJCB_Tail
			toReadyList->xJcbListItem.prev = (JCB_t *)pxReadyListJCB_Tail;
			if(pxReadyListJCB_Tail == NULL)
			{	//readyListJCB_Tail == NULL means readyListJCB_Head = NULL
				pxReadyListJCB = toReadyList;
			}
			else
			{
				pxReadyListJCB_Tail->xJcbListItem.next = (JCB_t *)toReadyList;
			}

			toReadyList->xJcbListItem.next = NULL;
			pxReadyListJCB_Tail = toReadyList;
			break;
		}
		else if(theJCB->uxPriority > toReadyList->uxPriority)
		{	// add toReadyList before theTCB
			toReadyList->xJcbListItem.next = (JCB_t *)theJCB;
			toReadyList->xJcbListItem.prev = theJCB->xJcbListItem.prev;
			theJCB->xJcbListItem.prev = (JCB_t *)toReadyList;

			if(theJCB == pxReadyListJCB)
			{
				pxReadyListJCB = toReadyList;
			}
			else
			{
				toReadyList->xJcbListItem.prev->xJcbListItem.next = (JCB_t *)
				                                                   toReadyList;
			}
			break;
		}
		else
		{
			theJCB = (JCB_t *)theJCB->xJcbListItem.next;
		}
	}while(1);
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
	return toReadyList;
}

JCB_t * pickReadyListJCB(JCB_t * pickJCB)
{
	JCB_t * theJCB;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

	if(pickJCB != NULL)
	{
		theJCB = (JCB_t *)pickJCB->xJcbListItem.next;
		if(theJCB == NULL)
		{	// the pickJCB is current readyListJCB_Tail, update it
			pxReadyListJCB_Tail = (JCB_t *)pickJCB->xJcbListItem.prev;
		}
		else
		{
			theJCB->xJcbListItem.prev = pickJCB->xJcbListItem.prev;
		}

		theJCB = (JCB_t *)pickJCB->xJcbListItem.prev;
		if(theJCB == NULL)
		{// the pickJCB is current readyListJCB_Head, update it
			pxReadyListJCB = (JCB_t *)pickJCB->xJcbListItem.next;
		}
		else
		{
			theJCB->xJcbListItem.next = pickJCB->xJcbListItem.next;
		}

	}
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
	return pickJCB;
}

JCB_t * pickHeadReadyListJCB()
{
	JCB_t * theJCB;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
	// theTCB = readyListJCB_Head;
	theJCB = pickReadyListJCB(pxReadyListJCB);
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
	return theJCB;
}
/// }}} readyListJCB }}}
