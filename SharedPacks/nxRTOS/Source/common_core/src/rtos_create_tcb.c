/**
  ******************************************************************************
  * @file           : rtos_create_tcb.c
  * @brief          : create TCB from a ready JCB
  ******************************************************************************
  * @attention
  * nxRTOS Kernel V0.0.1
  * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
  *
  * 1 tab == 2 spaces!
  */
/*-----------------------------------------------------------------------------
 * create a new TCB from a readyJCB
 *---------------------------------------------------------------------------*/

#include  "rtos_create_tcb.h"
#include  "rtos_jcb_ready_list.h"
#include  "rtos_tcb_live_list.h"
#include  "new_stackspace4thread.h"
#include  "arch4rtos_threadstack.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_mutex.h"
#include  "nxRTOSConfig.h"

// construct TCB from JCB
LiveTCB_t * rtos_create_tcb(JCB_t * pTheJCB)
{
  int  size;
  LiveTCB_t * theTCB = NULL;
  StackType_t * pStackPoint = NULL;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

  if(pTheJCB != NULL)
  {
    //  {{{  critical level raise   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(ThreadStackTempStacking == (pTheJCB->pThreadStack))
    { // for SHortLive_Thread

      size = pTheJCB->stackSize;  //  adjust size {{{
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
              << 	STACK_ALIGNMENT_BITS;   //  adjust size }}}
      pStackPoint = newStackSpace4Run2TermThread(size);
      // If success, get non-NULL pointer to first address from which can build
      // up ShortLive_TCB and stack_space for the new creating Thread.
      // for Stack_GrowLowAddr, the value expected to be a highest address
      // where to be available for Stack_Push operation.
      if(pStackPoint == NULL)
      { // this means no enough space left in SHortLiveThread_StackSpace
        // for a new ShortLiveThread to execute, try to promote
        // current--running--ShortLiveThread to expedite.
        // promote Run2TermTCB if(getCurrentRun2TermTCB() == NULL)
        if(getCurrentRun2TermTCB() != NULL)
        { // TODO, promote uxPriority through API in proper way
          getCurrentRun2TermTCB()->uxPriority = pTheJCB->uxPriority;
        }
        else  if(getCurrentRun2BlkTCB() != NULL)
        { // this is to prevent the pendingJCB to keep trying to preempt
          getCurrentRun2BlkTCB()->uxPriority = pTheJCB->uxPriority;
        }
      }
      else  //  if(pStackPoint != NULL)   //  the address, from which blow to
                                          //  construct  ShortLive_TCB
      { // TODO , extract theMutex from pTheJCB
        Mutex_t *   theMutex = getMutexInReadyJCB(pTheJCB);
#if   defined(STACK_GROW_TOLOWADDR) //  {{{ {{{
        { // adjust to make room for R2TTCB_t block from
          // (ShortLiveThread) StackSpace, pStackPoint point to highest address
          theTCB = (LiveTCB_t *)
                          ((AddressOpType_t)pStackPoint - sizeof(R2TTCB_t));
          theTCB = (LiveTCB_t *)((((AddressOpType_t)theTCB
                                      -(0x01 << STACK_ALIGNMENT_BITS) -1)
                                    >>  STACK_ALIGNMENT_BITS)
                                    <<  STACK_ALIGNMENT_BITS);
          // Actually theTCB is type of (R2TTCB_t *)
          theTCB->pxTopOfStack = (StackType_t *)theTCB -1;
        }
      //   }}}  defined(STACK_GROW_TOLOWADDR) }}}
#else ///  {{{ !defined(STACK_GROW_TOLOWADDR) {{{
        {
          //
          theTCB = (LiveTCB_t *)pStackPoint;
          theTCB = (LiveTCB_t *)((((AddressOpType_t)theTCB
                                      +(0x01 << STACK_ALIGNMENT_BITS) -1)
                                  >>  STACK_ALIGNMENT_BITS)
                                  <<  STACK_ALIGNMENT_BITS);
          pStackPoint = (StackType_t *)
                                  ((AddressOpType_t)theTCB + sizeof(R2TTCB_t));
          pStackPoint = (StackType_t *)((((AddressOpType_t)pStackPoint
                                          + (0x01 << STACK_ALIGNMENT_BITS) -1)
                                         >>  STACK_ALIGNMENT_BITS)
                                         <<  STACK_ALIGNMENT_BITS);
          // Actually theTCB is type of (R2TTCB_t *)
          theTCB->pxTopOfStack = (StackType_t *)pStackPoint;
// theTCB = theTCB;
          // Actually theTCB is type of (R2TTCB_t *)
        }
#endif  //  }}} !defined(STACK_GROW_TOLOWADDR)  }}}
        theTCB->stateOfTcb = TCB_RUN2TERM;
        // transfer the holding Mutex to Thread_context from JCB
        theTCB->pxMutexHeld = theMutex;
        if(theMutex != NULL)
        { // TODO, op through MutexAPI
          theMutex->owner_thread = (void *)theTCB;
          theMutex->pNextMutex = NULL;
        }
        // this update leave to invoker through pushRun2TermListTCB(theTCB)
        // but WHY can't do it here?
        // {
        //   theTCB->xTcbListItem.prev = getCurrentRun2TermTCB();
        //   getCurrentRun2TermTCB() = theTCB;
        // }
        ((R2TTCB_t *)theTCB)->xEventListItem.prev =NULL;
        ((R2TTCB_t *)theTCB)->xEventListItem.next =NULL;
        //((R2TTCB_t *)theTCB)->wFlags = 0;
        ((R2TTCB_t *)theTCB)->pxWaitingObj =NULL;
        ((R2TTCB_t *)theTCB)->xResumeOnTicks = 0;
      }
    }
    else  if(ThreadStackLongKeeping == (pTheJCB->pThreadStack))
    { // for LongLive_Thread
      size = pTheJCB->stackSize;
      if(!size)
      { // size == 0 , adjust to default minimal taskSize
        size = RTOS_MINIMAL_TASK_STACKSIZE;
      }
      // require system to allocate RAM for exclusive stack space
      // this will be traditional task style of Thread, but need system
      // to allocate the RAM space
      // theTCB = rtosAllocateThreadStack(size);
      theTCB = (LiveTCB_t *)newStackSpace4Run2BlckThread(size);
      if(theTCB != NULL)
      {
        Mutex_t *   theMutex = getMutexInReadyJCB(pTheJCB);
        theTCB->stateOfTcb = TCB_RUNNING; // can run forever
        // transfer the holding Mutex to Thread_context from JCB
        theTCB->pxMutexHeld = theMutex;
        if(theMutex != NULL)
        {
          theMutex->owner_thread = (void *)theTCB;
          theMutex->pNextMutex = NULL;
        }
        ((R2BTCB_t *)theTCB)->xEventListItem.prev =NULL;
        ((R2BTCB_t *)theTCB)->xEventListItem.next =NULL;
        ((R2BTCB_t *)theTCB)->wFlags = 0;
        ((R2BTCB_t *)theTCB)->pxWaitingObj =NULL;
        ((R2BTCB_t *)theTCB)->xResumeOnTicks = 0;
      }
    }
    else
    { // static or preallocated RAM through (readyJCB->pThreadStack)
      theTCB = (LiveTCB_t * )pTheJCB->pThreadStack; ;
      // Stack alignment needed
      {
        Mutex_t *   theMutex = getMutexInReadyJCB(pTheJCB);
        theTCB->stateOfTcb = TCB_RUNNING; // can run forever
        // transfer the holding Mutex to Thread_context from JCB
        theTCB->pxMutexHeld = theMutex;
        if(theMutex != NULL)
        {
          theMutex->owner_thread = (void *)theTCB;
          theMutex->pNextMutex = NULL;
        }
      }
      // performing alignment.
#if    defined(STACK_GROW_TOLOWADDR)
      { // make room for TCB
        void    * theSP = (void    *)theTCB + pTheJCB->stackSize;
        theSP = (void *)theSP - sizeof(LiveTCB_t);
        // Alignment to lower address,
        theSP = (StackType_t *)  (((StackType_t )theSP) >>  STACK_ALIGNMENT_BITS);
        theSP = (StackType_t *)  (((StackType_t )theSP) <<  STACK_ALIGNMENT_BITS);
        theTCB = (LiveTCB_t * )theSP;
      }
#else   //  STACK_GROW_TOHIGHADDR
      { // make room for TCB
        theSP = (void *)theSP  +  sizeof(LiveTCB_t);
        // Alignment to higher address,
        theSP = (StackType_t *)  (((StackType_t )theSP) + ((1 << STACK_ALIGNMENT_BITS) -1) );
        theSP = (StackType_t *)  (((StackType_t )theSP) >>  STACK_ALIGNMENT_BITS);
        theSP = (StackType_t *)  (((StackType_t )theSP) <<  STACK_ALIGNMENT_BITS);
        theTCB = (LiveTCB_t * )theSP;
      }
#endif
      theTCB->xTcbListItem.prev = getCurrentRun2BlkTCB();
      // this leave to pick_tcb
      //getCurrentRun2BlkTCB() = theTCB;
    }

    if(theTCB != NULL)
    { // fill in theTCB
      theTCB->pxJCB = pTheJCB;
      // initial top of Stack
  #if   defined(STACK_GROW_TOLOWADDR)
      //theTCB->pxTopOfStack = (StackType_t *)theTCB;
//      theTCB->pxTopOfStack = pStackPoint;
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
      //theTCB->xTcbListItem.prev =getCurrentRun2TermTCB();
      // or
      //theTCB->xTcbListItem.prev =getCurrentRun2BlkTCB();
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
      { // static allocated stack space
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
      { // something wrong
        while(1);
      }
      ///}}}	/// ( RTOS_THREAD_STACK_OVERFLOW_TRACE == 1 )	}}}
    #endif
  #endif
    }
    // }}}  critical level restored   }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  }
  return theTCB;
}
