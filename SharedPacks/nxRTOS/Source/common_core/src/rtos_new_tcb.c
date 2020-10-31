/* new_tcb.c
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
 * create a new TCB from a readyJCB
 *---------------------------------------------------------------------------*/
#include  "new_stackspace4thread.h"
#include  "list_jcb.h"
#include  "list_tcb.h"
#include  "arch4rtos_threadstack.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_new_tcb.h"
#include  "rtos_mutex.h"
#include  "nxRTOSConfig.h"


BaseTCB_t * createTCB(JCB_t * pTheJCB)
{
    int  size;
	BaseTCB_t * theTCB = NULL;
	StackType_t * pStackPoint = NULL;
	SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

	// critical section enter
	arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

	if(pTheJCB != NULL)
	{
		if(ThreadStackTempStacking == (pTheJCB->pThreadStack))
		{	// the Job will run in a Thread Context in SHortLiveThread_StackSpace
			// or StackSpace4Run2TermThread
			// share a RAM chunk for all run to termination threads
			size = pTheJCB->stackSize;

			if(!size)
			{	// size == 0 , adjust to default minimal taskSize
				size = RTOS_MINIMAL_THREAD_STACKSIZE;
				pTheJCB->stackSize = size;
			}
			// alignment to meet StackType_t
			size = ((size + (0x01 << STACK_ALIGNMENT_BITS) -1)
							>>  STACK_ALIGNMENT_BITS)
							<< 	STACK_ALIGNMENT_BITS;
			size += ((sizeof(R2TTCB_t) + (0x01 << STACK_ALIGNMENT_BITS) -1)
							>>  STACK_ALIGNMENT_BITS)
							<< 	STACK_ALIGNMENT_BITS;

			theTCB = (BaseTCB_t *)newStackSpace4Run2TermThread(size);
			// expect to get the position where to start fabric a new Thread_StackSpace
			// for Stack_GrowLowAddr, the return value expected to be a highest address
			// where to be available for Stack_Push operation.
			if(theTCB == NULL)
			{   // this means no enough space left in SHortLiveThread_StackSpace
				// for a new ShortLiveThread to execute, try to promote
				// current--running--ShortLiveThread to expedite.
				// promote Run2TermTCB if(pxRun2TermTCBList_Head == NULL)
			    if(pxRun2TermTCBList_Head != NULL)
			    {   // promote uxPriority
			        pxRun2TermTCBList_Head->uxPriority = pTheJCB->uxPriority;
			    }
			    else    if(pxRun2BlckTCBList_Head != NULL)
			    {   // this is to prevent the pendingJCB keep to try to preempt
			        pxRun2BlckTCBList_Head->uxPriority = pTheJCB->uxPriority;
			    }
			}
			else
			{   // TODO , init pxWaitingObj  etc from JCB
			    Mutex_t *   theMutex = pTheJCB->pxBindingObj;

#if    defined(STACK_GROW_TOLOWADDR)
			    // need to adjust to make room for R2TTCB_t block from
			    // (ShortLiveThread) StackSpace
			    pStackPoint = (StackType_t *)((AddressOpType_t)theTCB - size);
			    theTCB = (BaseTCB_t * )(pStackPoint + 1);
			    // Actually theTCB is type of (R2TTCB_t *)
#else
			    // all fine
			    pStackPoint = (StackType_t *)((AddressOpType_t)theTCB + size);
			    // theTCB = theTCB;
			    // Actually theTCB is type of (R2TTCB_t *)
#endif
			    theTCB->extTCBType = R2Term_TCB;
			    // transfer the holding Mutex to Thread_context from JCB
			    theTCB->pxOwnedObjList = theMutex;
			    if(theMutex != NULL)
			    {
			        theMutex->owner_job = NULL;
			        theMutex->owner_thread = (void *)theTCB;
			        theMutex->pxNextMutex = NULL;
			    }
			    theTCB->xStateListItem.prev = (ListXItem_t *)pxRun2TermTCBList_Head;
			    ((R2TTCB_t *)theTCB)->xEventListItem.prev =NULL;
			    ((R2TTCB_t *)theTCB)->xEventListItem.next =NULL;
			    ((R2TTCB_t *)theTCB)->wFlags = 0;
			    ((R2TTCB_t *)theTCB)->pxWaitingObj =NULL;
			    ((R2TTCB_t *)theTCB)->xResumeOnTicks = 0;
			}
			// this update leave to pick_tcb
			// pxRun2TermTCBList_Head = theTCB;
		}
		else  if(ThreadStackLongKeeping == (pTheJCB->pThreadStack))
		{
			size = pTheJCB->stackSize;

			if(!size)
			{	// size == 0 , adjust to default minimal taskSize
				size = RTOS_MINIMAL_TASK_STACKSIZE;
			}
			// require system to allocate RAM for exclusive stack space
			// this will be traditional task style of Thread, but need system
			// to allocate the RAM space
			// theTCB = rtosAllocateThreadStack(size);
            theTCB = (BaseTCB_t *)newStackSpace4Run2BlckThread(size);
            if(theTCB != NULL)
            {
                Mutex_t *   theMutex = pTheJCB->pxBindingObj;

                theTCB->extTCBType = R2Term_TCB;
                // transfer the holding Mutex to Thread_context from JCB
                theTCB->pxOwnedObjList = theMutex;
                if(theMutex != NULL)
                {
                    theMutex->owner_job = NULL;
                    theMutex->owner_thread = (void *)theTCB;
                    theMutex->pxNextMutex = NULL;
                }

                ((R2BTCB_t *)theTCB)->xEventListItem.prev =NULL;
                ((R2BTCB_t *)theTCB)->xEventListItem.next =NULL;
                ((R2BTCB_t *)theTCB)->wFlags = 0;
                ((R2BTCB_t *)theTCB)->pxWaitingObj =NULL;
                ((R2BTCB_t *)theTCB)->xResumeOnTicks = 0;
            }
		}
		else
		{	// static or preallocated RAM through (readyJCB->pThreadStack)
			theTCB = (BaseTCB_t * )pTheJCB->pThreadStack; ;
			// Stack alignment needed
			{
                Mutex_t *   theMutex = pTheJCB->pxBindingObj;

                theTCB->extTCBType = R2Term_TCB;
                // transfer the holding Mutex to Thread_context from JCB
                theTCB->pxOwnedObjList = theMutex;
                if(theMutex != NULL)
                {
                    theMutex->owner_job = NULL;
                    theMutex->owner_thread = (void *)theTCB;
                    theMutex->pxNextMutex = NULL;
                }
			}
		    // performing alignment.
		#if    defined(STACK_GROW_TOLOWADDR)
		    {   // make room for TCB
			    void    * theSP = (void    *)theTCB + pTheJCB->stackSize;
		        theSP = (void *)theSP - sizeof(BaseTCB_t);
		        // Alignment to lower address,
		        theSP = (StackType_t *)  (((StackType_t )theSP) >>  STACK_ALIGNMENT_BITS);
		        theSP = (StackType_t *)  (((StackType_t )theSP) <<  STACK_ALIGNMENT_BITS);
		        theTCB = (BaseTCB_t * )theSP;
		    }
		#else   //  STACK_GROW_TOHIGHADDR
		    {   // make room for TCB
		        theSP = (void *)theSP  +  sizeof(BaseTCB_t);
		        // Alignment to higher address,
		        theSP = (StackType_t *)  (((StackType_t )theSP) + ((1 << STACK_ALIGNMENT_BITS) -1) );
		        theSP = (StackType_t *)  (((StackType_t )theSP) >>  STACK_ALIGNMENT_BITS);
		        theSP = (StackType_t *)  (((StackType_t )theSP) <<  STACK_ALIGNMENT_BITS);
                theTCB = (BaseTCB_t * )theSP;
		    }
		#endif
			theTCB->xStateListItem.prev =(ListXItem_t *)pxRun2BlckTCBList_Head;
			// this leave to pick_tcb
			//pxRun2BlckTCBList_Head = theTCB;
		}
	}

	if(theTCB != NULL)
	{   // fill in theTCB
	    theTCB->pxJCB = pTheJCB;
	    // initial top of Stack
#if     defined(STACK_GROW_TOLOWADDR)
	    //theTCB->pxTopOfStack = (StackType_t *)theTCB;
	    theTCB->pxTopOfStack = pStackPoint;
#else
        theTCB->pxTopOfStack = (StackType_t *)((void *)theTCB + sizeof(theTCB));
        // alignment here
        theTCB->pxTopOfStack = (StackType_t )(theTCB->pxTopOfStack)
        							+ ((1 << STACK_ALIGNMENT_BITS) -1) );
        theTCB->pxTopOfStack = (StackType_t *)
        		(((StackType_t )(theTCB->pxTopOfStack))
        				>>  STACK_ALIGNMENT_BITS);
        theTCB->pxTopOfStack = (StackType_t *)
        		(((StackType_t )(theTCB->pxTopOfStack))
        				<<  STACK_ALIGNMENT_BITS);
#endif
	    theTCB->uxPriority = pTheJCB->uxPriority;
	    //theTCB->xStateListItem.prev =pxRun2TermTCBList_Head;
	    // or
	    //theTCB->xStateListItem.prev =pxRun2BlckTCBList_Head;
#ifdef  RTOS_THREAD_STACK_OVERFLOW_TRACE
/// {{{
#if    defined(STACK_GROW_TOLOWADDR)
    /// {{{
    if((ThreadStackTempStacking == pTheJCB->pThreadStack)
       ||
	   (ThreadStackLongKeeping == pTheJCB->pThreadStack))
    {
        theTCB->pxStackMarkPosition = (StackType_t *)
                ((void *)theTCB - pTheJCB->stackSize);
        // aka. = (StackType_t *)
        //  ((void *)(theTCB->pxTopPfStack + 1) - pTheJCB->stackSize);
    }
    // #if track stack overflow for static allocated stack
    else
    {   // static allocated stack space
        theTCB->pxStackMarkPosition = (StackType_t *)
                ((void *)theTCB->pxTopOfStack - size + sizeof(StackType_t));
    }
    ///     (STACK_GROW_TOLOWADDR) }}}
#else
    ///     {{{
    if(pTheJCB->pThreadStack == ThreadStackTempStacking)
    {
        theTCB->pxStackMarkPosition = (StackType_t *)
                ((void *)theTCB->pxTopOfStack +  size);
    }else if(ThreadStackLongKeeping == pTheJCB->pThreadStack)
    {
        theTCB->pxStackMarkPosition = (StackType_t *)
                ((void *)theTCB->pxTopOfStack + size);
    }
    // #if track stack overflow for static allocated stack
    else
    {   // static allocated stack space
        theTCB->pxStackMarkPosition = (StackType_t *)
                ((void *)theTCB->pxTopOfStack + size - sizeof(StackType_t));
    }

/// }}}
#endif
/// RTOS_THREAD_STACK_OVERFLOW_TRACE    }}}


	//  prepare thread_to_isr_frame and other things in thread_stack.
	//  refer to pendsv_handler for contains in thread_stack.
	theTCB->pxTopOfStack = arch4rtos_pInitThreadStack
	                      	  ((StackType_t *)theTCB->pxTopOfStack, pTheJCB );
#if ( RTOS_THREAD_STACK_OVERFLOW_TRACE == 1 )
///{{{													{{{
	if(NULL != theTCB->pxStackMarkPosition)
	{
	    *(theTCB->pxStackMarkPosition) = RTOS_STACK_OVERFLOW_TRACE_DEFAULTMARK;
	}
	else
	{   // something wrong
	    while(1);
	}
///}}}	/// ( RTOS_THREAD_STACK_OVERFLOW_TRACE == 1 )	}}}
#endif

#endif

	}
	// critical section exit
	arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
	return theTCB;
}
