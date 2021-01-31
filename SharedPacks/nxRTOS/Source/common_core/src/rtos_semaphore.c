/**
  ******************************************************************************
  * @file           : rtos_semaphore.c
  * @brief          :
  * ****************************************************************************
  * @attention
  *                 nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */

/*-----------------------------------------------------------------------------
 * Semaphore control block
 *---------------------------------------------------------------------------*/
#include  "rtos_semaphore.h"
#include  "rtos_sema_free_list.h"
#include  "rtos_sema_waiting_list.h"
#include  "rtos_sema_tentative_jcb_list.h"
#include  "rtos_defer_tcb.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_jcb.h"
#include  "rtos_jcb_ready_list.h"
#include  "rtos_tcb_live_list.h"
#include  "nxRTOSConfig.h"
/**
  * @brief  Create and initialize a Semaphore Control Block.
  *         This function is invoked when a new Semaphore creates.
  * @param  theSem, pointer to the Semaphore Control Block
  *                 when is NULL, means to require allocate memory from
  *                 designated system FreeSemControlBlock_MemPool.
  *         max_count, counted Semaphore supported, the maximum token allowed
  *                 for the semaphore. To binary semaphore it to be 1.
  *         initial_count, the initial token value for the semaphore to be
  *                 created and/or initialized.
  * @retval TBD. error code
  */
#if  (SUPPORT_SEMA_ATTRIBUTES)
Sema_t * pxSemNew(Sema_t * theSem, uint32_t max_count,
                  uint32_t initial_count, const osSemaAttr_t * pSema_attr)
#else
Sema_t * pxSemNew(Sema_t * theSem, uint32_t max_count, uint32_t initial_count)
#endif
{
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

  // {{{  critical section enter {{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  if(NULL == theSem)
  {
    theSem = xpickSemFromFreeList();
  }
  if(NULL != theSem)
  {
    theSem->pNextSema = NULL;
    theSem->pxWaitingBaseTCBList = NULL;
    theSem->max_tokens = max_count;
    theSem->tokens = initial_count;
#if  (SUPPORT_SEMA_ATTRIBUTES)
    if (NULL == pSema_attr)
    {
      theSem->sema_attr = osSemaAttrAllClear; //0
    }
    else
    {
      theSem->sema_attr = (*pSema_attr) & osSemaAttrAllSet; // attr
    }
#endif
  }
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // }}}  critical section exit  }}}
  return  theSem;
}
/**
  * @brief  put Semaphore Control Block back into sysSemFreeList..
  * @param  theSem, pointer to the Semaphore Control Block.
  *         It need application to guarante there is no Job or liveThread
  *         pending for the Semaphore and none of application code will
  *         release/send the Semaphore.
  * @retval TBD. error code
  */
int xSemDelete(Sema_t * theSem)
{
  int retV = 0;
  if(theSem != NULL)
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // {{{ critical section enter {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    pxInsertSemToFreeList(theSem);
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}  critical section exit  }}}
  }
  return retV;
}
/**
  * @brief  intend to increase token count in Semaphore Control Block.
  *         if previously the token count is 0 and a Job or liveThread is
  *         waiting the semaphore, the Semaphore may deliver to the competitor
  *         (waiter) and may check with kernel scheduler if thread switch over
  *         needed.
  * @param  theSem, pointer to the Semaphore Control Block.
  * @retval osStatus_t， status code that indicates the execution status of
  *         the function. usually no use
  */
int xSemRelease(Sema_t * theSem)
{
  int retV = 0;
  if(theSem != NULL)
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // {{{  critical section enter  {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    theSem->tokens ++;
    if(theSem->tokens > theSem->max_tokens)
    {
       theSem->tokens = theSem->max_tokens;
    }
    deliverSema(theSem);
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}  critical section exit  }}}
  }
  return retV;
}
/**
  * @brief  Blocking function to compete for acquire a Semaphore.
  *         This function only can be invoke from a LongliveThread context.
  *         can't to be invoked from neither an ISR context nor ShortliveThread
  *         context.
  * @param  acqSem, pointer to the Semaphore Control Block.
  *         timeout, the timeout in sys ticks to unblock if can't acquire
  *         the Semaphore.
  * @retval TBD. error code
  */
Sema_t * pxSemAcquire(Sema_t * acqSem, TickType_t timeout)
{
  Sema_t * theSem = acqSem;
  if(theSem != NULL)
  {
    R2BTCB_t  *  theTCB;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    theTCB = (R2BTCB_t   *)getCurrentTCB();
    if(acqSem->tokens)
    {
      acqSem->tokens --;
    }
    else  if(timeout == 0)
    { // for timeout == 0 , it immediately  return NULL
      theSem = NULL;
    }
    else  //    timeout != 0
    {   // add theTCB into waitingList
      if(theTCB != NULL)
      {
        removeTCBFromRun2BlckTCBList((LiveTCB_t *)theTCB);
        theTCB->pxWaitingObj = acqSem;
        //addTCBToSemWaitingList(acqSem, theTCB);
        pxInsertToSemWaitList(acqSem, (BaseTCB_t *)theTCB);
        if(timeout == WAIT_FOREVER)
        {
          theTCB->wFlags = Wait4Sem;
          theTCB->baseTCB.xTcbListItem.next = NULL;
          theTCB->baseTCB.xTcbListItem.prev = NULL;
        }
        else
        {   // add theTCB to deferTCB as well
          theTCB->xResumeOnTicks = timeout; // != 0
          theTCB->wFlags = Wait4Ticks  | Wait4Sem;
          addToDeferTCBWaitingList(theTCB);
        }
        // getCurrentTCB() went to waiting list, need to
        arch4rtosReqSchedulerService();
      }
      else
      {
        while(1);
      }
    }
    // after this point it means the Thread resumed if it has been
    //  in one or more waitingList
    if(theSem != NULL)
    {   // check the resume reason
      if(theTCB->wFlags & Wait4Ticks)
      {   // the flag of Wait4Ticks is set so it is from timeout
        theSem = NULL;  // return NULL to indicate timeout
        theTCB->wFlags ^= Wait4Ticks;
      }
    }
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  }
  return theSem;
}
/**
  * @brief  get token count in Semaphore Control Block.
  *         This is a function for peek in ISR or ShortliveThread context
  *         usually.
  * @param  theSem, pointer to the Semaphore Control Block.
  * @retval TBD. error code
  */
int xSemGetCount(Sema_t * theSem)
{
  int retV = 0;
  if(theSem != NULL)
  {
    retV = theSem->tokens;
  }
  return  retV;
}

/**
  * @brief  utility function walk through to deliver Semaphore to JCB or liveTCB
  *         which in the waitingList.
  *         Invoke on the time
  *         sema->token increased or pxTentativeToJob is cleared.
  * @param  theSem, pointer to the Semaphore Control Block.
  * @retval TBD. error code
  */
int deliverSema(Sema_t * theSem)
{
  if(theSem != NULL)
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // {{{  critical section enter  {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    // walk through WaitingList  if tokens > 0
    while ((theSem->tokens > 0) && (theSem->pxWaitingBaseTCBList != NULL))
    {
      if(JCB_WAITINGSEMAG == theSem->pxWaitingBaseTCBList->stateOfJcb)
      { // this is a JCB at Top_Head in pxWaitingBaseTCBList
        JCB_t * theJCB; //= (JCB_t *)theSem->pxWaitingBaseTCBList;
        theSem->tokens --;
        //theJCB = (JCB_t *)pickHeadSemWaitList(theSem);
        theJCB = (JCB_t *)theSem->pxWaitingBaseTCBList;
        theSem->pxWaitingBaseTCBList = theJCB->xBTcbListItem.next;
        if(NULL !=  theSem->pxWaitingBaseTCBList)
        {
          theSem->pxWaitingBaseTCBList->xBTcbListItem.prev = NULL;
        }
        {
          theJCB->pxBindingObj = NULL;
          theJCB->stateOfJcb = JCB_STATE_READY;
        }
        // add the JCB to ReadyList
        pxInsertToReadyJCBList(theJCB);
        // for new JCB into readyJCBList, check if SchedulerService needed
        arch4rtosReqSchedulerService();
      }
      else if(TCB_WAITINGSEMA == theSem->pxWaitingBaseTCBList->stateOfTcb)
      { // this is a TCB the Top_Head in pxWaitingBaseTCBList
        R2BTCB_t * theTCB = (R2BTCB_t *)
                                    theSem->pxWaitingBaseTCBList->stateOfJcb;
        theSem->tokens --;
        pickHeadSemWaitList(theSem);
#if     1
        //  if support timer guarded semaphore acquisition
        if((theTCB->wFlags & Wait4Ticks)  != 0)
        {  // the TCB is in TimeredWatingList
           // pick it out
           pickFromDeferTCBWaitingList(theTCB);
           // clear the flag of Wait4Ticks to indicate the Thread _resume
           // from a waiting Object  acquired, rather than timeout
           theTCB->wFlags ^= Wait4Ticks;
        }
#endif
        insertTCBToRun2BlckTCBList((LiveTCB_t *)theTCB);
        if((getCurrentTCB() != NULL) &&
               (theTCB->baseTCB.uxPriority < getCurrentTCB()->uxPriority))
        {
         arch4rtosReqSchedulerService();
        }
        //continue;
      }
      else
      { // something wrong here
        while(1);
      }
    }
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}  critical section exit  }}}
  }
  return 0;
}
