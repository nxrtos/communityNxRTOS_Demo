/*
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

#if	(RTOS_SYS_JCB_NUM != 0)
static JCB_t	preallocJCB[RTOS_SYS_JCB_NUM];
#endif


/// {{{ freeListJCB {{{
// pre_allocated list of JBC for freeListJCB
// freeListJCB_Head for pick a JCB from freeList
// freeListJCB_Tail for put/recycle a JCB to freeList

JCB_t * freeListJCB_Head = NULL;
JCB_t * freeListJCB_Tail = NULL;

JCB_t * xInitfreeListJCB()
{
	SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

	// {{{ critical section start {{{
	arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

#if (RTOS_SYS_JCB_NUM != 0)
	if(freeListJCB_Head == NULL)
	{
		for(int i = 0; i < RTOS_SYS_JCB_NUM -1; i++)
		{
			preallocJCB[i].xJcbListItem.next = (JCB_t *)&preallocJCB[i+1];
		}

		freeListJCB_Tail = &preallocJCB[RTOS_SYS_JCB_NUM -1];
		freeListJCB_Tail->xJcbListItem.next = NULL;

		freeListJCB_Head = &preallocJCB[0];
	}
#endif

	arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}} critical section end }}}
	return freeListJCB_Head;
}

JCB_t * xappendFreeListJCB(JCB_t * toFreeList)
{
	JCB_t * theJB = toFreeList;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
	if(theJB != NULL)
	{
		toFreeList->xJcbListItem.next = NULL;
		if(freeListJCB_Head == NULL)
		{	// if the freeList is empty now
			freeListJCB_Tail = toFreeList;
			freeListJCB_Head = toFreeList;
			toFreeList->xJcbListItem.prev = NULL;
		}
		else
		{
			freeListJCB_Tail->xJcbListItem.next = (JCB_t *)toFreeList;
			toFreeList->xJcbListItem.prev = (JCB_t *)freeListJCB_Tail;
			freeListJCB_Tail = toFreeList;
		}

	}
    // critical section exit    }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
	return theJB;
}

JCB_t * xpickFreeListJCB()
{
	JCB_t * theJCB;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
	theJCB = freeListJCB_Head;
	if(theJCB != NULL)
	{
		freeListJCB_Head = (JCB_t *) theJCB->xJcbListItem.next;
		if(freeListJCB_Head == NULL)
		{   // if freeListJCB empty, update freeListJCB_Tail
		    freeListJCB_Tail = NULL;
		}
		theJCB->xJcbListItem.next = NULL;
	}
    // critical section exit    }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
	return theJCB;
}
/// }}} freeListJCB }}}


/// {{{ run2TermListJCB {{{
// running to termination JCB list
// run2TermList may kept in TCB rather than JCB.
// As the Thread may change its priority along with processing

JCB_t * run2TermListJCB_Head = NULL;
JCB_t * pushRun2TermListJCB(JCB_t * toList)
{
	JCB_t * theJCB = NULL;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
	if(run2TermListJCB_Head == NULL)
	{
		run2TermListJCB_Head = toList;
		theJCB = toList;
		theJCB->xJcbListItem.next = NULL;
		theJCB->xJcbListItem.prev = NULL;
	}
	else if(run2TermListJCB_Head->uxPriority > toList->uxPriority)
	{	// the toList with higher priority than run2TermListJCB_Head
		theJCB = toList;
		theJCB->xJcbListItem.next = (JCB_t *)run2TermListJCB_Head;
		theJCB->xJcbListItem.prev = NULL;
		run2TermListJCB_Head->xJcbListItem.prev = (JCB_t *)theJCB;
		run2TermListJCB_Head = toList;
	}
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
	return theJCB;
}

JCB_t * popRun2TermListJCB()
{
	JCB_t * theJCB;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
	theJCB = run2TermListJCB_Head;
	if(theJCB != NULL)
	{	// the toList with higher priority than run2TermListJCB_Head
		run2TermListJCB_Head = (JCB_t *)theJCB->xJcbListItem.next;
		if(run2TermListJCB_Head != NULL)
		{
			run2TermListJCB_Head->xJcbListItem.prev = NULL;
		}
	}
    // critical section exit    }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
	return theJCB;
}

/// }}} run2TermListJCB }}}

JCB_t * xInitListJCB(void)
{
    JCB_t * theJCB;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    //xInitfreeListJCBC();
    // init freeSoftTimerList if support deferJob
	theJCB = xInitfreeListJCB();
    initReadyListJCB();

    // critical section exit    }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);

	return theJCB;
}
