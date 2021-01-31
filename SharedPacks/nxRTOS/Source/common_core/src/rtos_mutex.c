/**
  ******************************************************************************
  * @file           : rtos_mutex.c
  * @brief          :
  * ****************************************************************************
  * @attention
  *                 nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */

/*-----------------------------------------------------------------------------
 * Mutex Control Block
 *---------------------------------------------------------------------------*/
#include  "rtos_mutex.h"
#include  "rtos_mutex_free_list.h"
#include  "rtos_mutex_waiting_list.h"
#include  "rtos_mutex_holding_list.h"
#include  "rtos_defer_tcb.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_jcb.h"
#include  "rtos_jcb_ready_list.h"
#include  "rtos_tcb_live_list.h"
#include  "nxRTOSConfig.h"
/**
  * @brief  Create and initialize a Mutex Control Block.
  *         This function is invoked when a new mutex creates.
  * @param  theMutex, pointer to the Mutex Control Block
  *                 when is NULL, means to require allocate memory from
  *                 designated system FreeMutexControlBlock_MemPool.
  *         max_count, counted mutex supported, the maximum token allowed
  *                 for the mutex. To binary mutex it to be 1.
  *         initial_count, the initial token value for the mutex to be
  *                 created and/or initialized.
  * @retval theMutex, point to Mutex Control Block,
  *         or error code, indicate operation failed and the cause etc.
  */
#if  (SUPPORT_MUTEX_ATTRIBUTES)
Mutex_t * pxMutexNew(Mutex_t * theMutex,osMutexAttr_t * pMutexAttr )
#else
Mutex_t * pxMutexNew(Mutex_t * theMutex)
#endif
{
  if(NULL == theMutex)
  { // need to get mem for Mutex Control Block
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // {{{  critical section enter {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    theMutex = xpickMutexFromFreeList();
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}  critical section exit  }}}
  }
  else
  { // for pre-existing Mutex Control Block, may have sanity check here.
  }
  if(NULL != theMutex)
  { // have valid memory block for Mutex Control Block
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // {{{  critical section enter {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    theMutex->pNextMutex = NULL;
    theMutex->pxWaitingBaseTCBList = NULL;
    theMutex->owner_thread = NULL;
#if  (SUPPORT_MUTEX_ATTRIBUTES)
    if(NULL == pMutexAttr)
    { // NULL of pMutexAttr treated as osMutexAttrAllClear
      theMutex->mutex_attr = osMutexAttrAllClear;
    }
    else
    { // sanity clean unused bit to input given
      theMutex->mutex_attr = *pMutexAttr & osMutexAttrAllSet;
    }
#endif
#if  (SUPPORT_MUTEX_ATTR_RECURSIVE)
    theMutex->lock = 0; // track recursive count, ref to osRtxMutex_t
#endif
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}  critical section exit  }}}
  }
  return  theMutex;
}
/**
  * @brief  put Mutex Control Block back into sysMutexFreeList..
  * @param  theMutex, pointer to the Mutex Control Block.
  *         It need application to guarante there is no Job or liveThread
  *         pending for the mutex and none of application code will
  *         release/send the mutex.
  * @retval TBD. error code
  */
int xMutexDelete(Mutex_t * theMutex)
{
  int retV = 0;
  // add senity check here if wanted. make sure the Mutex is good to be deleted.
  // e.g. no waiting Job/Thread, no owner Job/Thread to the Mutex
  if(theMutex != NULL)
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // {{{ critical section enter {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    pxInsertMutexToFreeList(theMutex);
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}  critical section exit  }}}
  }
  return retV;
}
/**
  * @brief  intend to increase token count in Mutex Control Block.
  *         if previously the token count is 0 and a Job or liveThread is
  *         waiting the mutex, the mutex may deliver to the competitor
  *         (waiter) and may check with kernel scheduler if thread switch over
  *         needed.
  * @param  theMutex, pointer to the Mutex Control Block.
  * @retval osStatus_t， status code that indicates the execution status of
  *         the function. usually no use
  */
int xMutexRelease(Mutex_t * theMutex)
{
  int retV = 0;
  if(theMutex != NULL)
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // {{{  critical section enter  {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(theMutex->owner_thread == getCurrentTCB())
    { //against owner to current TCB for sanity test.
#if  (SUPPORT_MUTEX_ATTR_RECURSIVE) //  {{{ {{{
      if((osMutexRecursive & theMutex->mutex_attr) && (theMutex->lock /*!= 0*/))
      { // for theMutex->lock != 0, only
        theMutex->lock --;
      }// when SUPPORT_MUTEX_ATTR_RECURSIVE
      else
#endif  //  }}} #if  (SUPPORT_MUTEX_ATTR_RECURSIVE) }}}
      { // clear owner_thread, when either (theMutex->lock == 0)
        //                    or !(SUPPORT_MUTEX_ATTR_RECURSIVE)
        // remove Mutex from theTCB->pxMutexHeld List
        pxRemoveMutexFromHeldBinding(theMutex);
        theMutex->owner_thread = NULL;
        distributeMutex(theMutex);
      }
    }
    else  //if(theMutex->owner_thread != getCurrentTCB())
    { // xMutexRelease() to be invoked from a LiveThread which is not the owner
      // catch error or just skip
      while(1);
    }
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}  critical section exit  }}}
  }
  return retV;
}
/**
  * @brief  Blocking function to compete for acquire a mutex.
  *         Usually this function to be invoked from a LongliveThread context.
  *         For ShortliveThread only 0 timeout be allowed.
  *         it can't to be invoked from an ISR context.
  *         TODO, complete later.
  * @param  acqMutex, pointer to the Mutex Control Block.
  *         timeout, the timeout in sys ticks to unblock if can't acquire
  *         the mutex.
  * @retval TBD. error code
  */  //  TODO, complete later.
Mutex_t * pxMutexAcquire(Mutex_t * acqMutex, TickType_t timeout)
{
  Mutex_t * theMutex = acqMutex;
  if(theMutex != NULL)
  {
    LiveTCB_t * theTCB = (LiveTCB_t *)getCurrentTCB();
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(acqMutex->owner_thread == NULL)
    {
      acqMutex->owner_thread = theTCB;
      // add acqMutex into theTCB-> -> pxMutexHeld List

#if  (SUPPORT_MUTEX_ATTR_RECURSIVE) //  {{{ {{{
      if((osMutexRecursive & theMutex->mutex_attr))
      {
        theMutex->lock = 0;
      }
#endif  //  }}} #if  (SUPPORT_MUTEX_ATTR_RECURSIVE) }}}
    }
#if  (SUPPORT_MUTEX_ATTR_RECURSIVE) //  {{{ {{{
    if((osMutexRecursive & theMutex->mutex_attr)
        && (acqMutex->owner_thread == getCurrentTCB()))
    {
      theMutex->lock ++;
      // theMutex = acqMutex; indicate acquire success
    }
#endif  //  }}} #if  (SUPPORT_MUTEX_ATTR_RECURSIVE) }}}
    else  if(timeout == 0)
    { // for timeout == 0 , it immediately  return NULL
      theMutex = NULL;
    }
    else  //    timeout != 0
    {   // add theTCB (must be a LongliveTCB) into waitingList
      R2BTCB_t * theLongliveTCB = (R2BTCB_t *)theTCB;
      if(theTCB != NULL)
      {
        removeTCBFromRun2BlckTCBList((LiveTCB_t *)theTCB);
        theLongliveTCB->pxWaitingObj = acqMutex;
        //addTCBToMutexWaitingList(acqMutex, theTCB);
        pxInsertToMutexWaitList(acqMutex, (BaseTCB_t *)theTCB);
        if(timeout == WAIT_FOREVER)
        {
          theLongliveTCB->wFlags = Wait4Mutex;
          theLongliveTCB->baseTCB.xTcbListItem.next = NULL;
          theLongliveTCB->baseTCB.xTcbListItem.prev = NULL;
        }
        else
        {   // add theTCB to deferTCB as well
          theLongliveTCB->xResumeOnTicks = timeout; // != 0
          theLongliveTCB->wFlags = Wait4Ticks  | Wait4Mutex;
          addToDeferTCBWaitingList(theLongliveTCB);
        }
        // getCurrentTCB() went to waiting list, need to
        arch4rtosReqSchedulerService();
      }
      else
      {
        while(1);
      }
      // after this point it means the Thread resumed if it has been
      //  in one or more waitingList
      if(theMutex != NULL)
      {   // check the resume reason
        if(theLongliveTCB->wFlags & Wait4Ticks)
        {   // the flag of Wait4Ticks is set so it is from timeout
          theMutex = NULL;  // return NULL to indicate timeout
          theLongliveTCB->wFlags ^= Wait4Ticks;
        }
      }
    }
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  }
  return theMutex;
}
/**
  * @brief  get owner Thread of the Mutex.
  *         This is a function may used for peek in ISR or ShortliveThread
  *         context. It may need to raise sysCriticalLevel to rtosCriticalLevel
  *         before invoke this function to guarantee the return to be valid
  *         through the time to get it.
  * @param  theMutex, pointer to the Mutex Control Block.
  * @retval theTCB, point to a LiveTCB_t structure,
  *                 or NULL for no owner_thread at this moment.
  */
LiveTCB_t * pxMutexGetOwnerThread(Mutex_t * theMutex)
{
  LiveTCB_t * theTCB = NULL;
  if(theMutex != NULL)
  {
    theTCB = theMutex->owner_thread;
    if((NULL != theTCB) && (JCB_STATE_READY_WITHMUTEX == theTCB->stateOfJcb))
    { // rule out JCB_STATE_READY_WITHMUTEX
      theTCB = NULL;
    }
    else
    {
      // theTCB = theMutex->owner_thread;
    }
  }
  return  theTCB;
}
/**
  * @brief  get owner Job of the Mutex.
  *         This is a function may used for peek in ISR or ShortliveThread
  *         context. It may need to raise sysCriticalLevel to rtosCriticalLevel
  *         before invoke this function to guarantee the return to be valid
  *         through the time to get it.
  * @param  theMutex, pointer to the Mutex Control Block.
  * @retval theJCB, point to a JCB_t structure,
  *                 or NULL for no owner_Job at this moment.
  */
JCB_t * pxMutexGetOwnerJob(Mutex_t * theMutex)
{
  JCB_t * theJCB = NULL;
  if(theMutex != NULL)
  {
    {
      LiveTCB_t * theTCB = theMutex->owner_thread;
      if((NULL != theTCB) && (JCB_STATE_READY_WITHMUTEX == theTCB->stateOfJcb))
      { // actually in owner_thread is a Job
        theJCB = (JCB_t *)theTCB;
      }
      else
      {
        // theJCB = NULL;
      }
    }
  }
  return  theJCB;
}
/**
  * @brief  get owner of the Mutex.
  *         This is a function for peek in ISR or ShortliveThread context
  *         usually. This is likely to be
  *         pxMutexGetOwnerThread | pxMutexGetOwnerJob
  *         before invoke this function to guarantee the return to be valid
  *         through the time to get it.
  * @param  theMutex, pointer to the Mutex Control Block.
  * @retval theTCB, point to owner_thread, or pxTentativeToJob
  *                 or NULL
  */
BaseTCB_t * xMutexGetOwner(Mutex_t * theMutex)
{
  BaseTCB_t *  theTCB = NULL;
  if(theMutex != NULL)
  {
    theTCB = (BaseTCB_t *)theMutex->owner_thread;
  }
  return  theTCB;
}
/**
  * @brief  utility function walk through to deliver mutex to JCB or liveTCB
  *         which in the waitingList.
  *         Invoke on the time mutex released, or mutex_aquire when mutex is
  *         available. This utility function may better to implement as a inline
  *         or direct to use a subset processing as some pre-condition may
  *         resolved at the caller's context.
  *         This implementation may used as exhausted super set processing, to
  *         reference to other sub-set usage.
  * @param  theMutex, pointer to the Mutex Control Block.
  * @retval TBD. error code
  */
int distributeMutex(Mutex_t * theMutex)
{
  if(theMutex != NULL)
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // {{{  critical section enter  {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    // walk through WaitingList  if tokens > 0
    if ((theMutex->owner_thread == NULL)
        && (theMutex->pxWaitingBaseTCBList != NULL))
    {
      if(JCB_WAITINGMUTEX == theMutex->pxWaitingBaseTCBList->stateOfJcb)
      { // this is a JCB the Top_Head in pxWaitingBaseTCBList
        JCB_t * theJCB = (JCB_t *)theMutex->pxWaitingBaseTCBList;
        { // deliver theJCB to owner_thread
          theJCB = (JCB_t *)pickHeadMutexWaitList(theMutex);
          theMutex->owner_thread = (LiveTCB_t *)theJCB;
          theJCB->stateOfJcb = JCB_STATE_READY_WITHMUTEX;
          theJCB->pxBindingObj = theMutex;
          // insert to readyJCBList
          pxInsertToReadyJCBList(theJCB);
          if((getCurrentTCB() != NULL) &&
                 (theJCB->uxPriority < getCurrentTCB()->uxPriority))
          {
           arch4rtosReqSchedulerService();
          }
        }
      }
      else
      if(TCB_WAITINGMUTEX == theMutex->pxWaitingBaseTCBList->stateOfTcb)
      { // this is a LiveTCB the Top_Head in pxWaitingBaseTCBList
        //TODO, finish later
        R2BTCB_t * theTCB = (R2BTCB_t *)pickHeadMutexWaitList(theMutex);
#if     1   //{{{
        //  if support timer guarded mutex acquisition
        if((theTCB->wFlags & Wait4Ticks)  != 0)
        {  // the TCB is in TimeredWatingList
           // pick it out
           pickFromDeferTCBWaitingList(theTCB);
           // clear the flag of Wait4Ticks to indicate the Thread _resume
           // from a waiting Object  acquired, rather than timeout
           theTCB->wFlags ^= Wait4Ticks;
        }
#endif  //  }}}
        insertTCBToRun2BlckTCBList((LiveTCB_t *)theTCB);
        if((getCurrentTCB() != NULL) &&
               (theTCB->baseTCB.uxPriority < getCurrentTCB()->uxPriority))
        {
         arch4rtosReqSchedulerService();
        }
      }
      else
      { // something wrong here
        while(1);
      }
    }
    else
    {
      // no distribution to do as Mutex held by owner_thread
      // or no WaitingBaseTCB
    }
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}  critical section exit  }}}
  }
  return 0;
}
/**
  * @brief  get Mutex handler from a ready JCB.
  * @param  theJCB: pointer to a structure of JCB_t
  * @retval the Mutex handler point to a structure of Mutex_t
  *         NULL if no Mutex has hold by theJCB
  */
Mutex_t * getMutexInReadyJCB(JCB_t * theJCB)
{
  Mutex_t * theMutex = NULL;
  if((theJCB->stateOfJcb == JCB_STATE_READY_WITHMUTEX))
  {
    theMutex = theJCB->pxBindingObj;
    if(NULL == theJCB->pxBindingObj)
    { // just a sanity check
      while(1);
    }
  }
  return theMutex;
}
/**
  * @brief  utility function to clear mutex had been tentative to give a JCB.
  *         This function provided when SUPPORT_MUTEX_ATTR_JOB_REASSIGN,
  *         or when cancel a JCB with a Mutex.
  *         More act as Macro.
  * @param  theJCB, pointer to the Job Control Block.
  * @retval the pointer to the Mutex
  */
Mutex_t * pxPickMutexInReadyJCB(JCB_t * theJCB)
{
  Mutex_t * theMutex = NULL;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // {{{  critical section enter {{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  if((NULL != theJCB) && (JCB_STATE_READY_WITHMUTEX == theJCB->stateOfJcb))
  {
    theMutex = theJCB->pxBindingObj;
    if (NULL != theMutex)
    { // something wrong
      while(1);
    }
    { // the Mutex has attribute does NOT support osMutexJobReassign,
      theJCB->pxBindingObj = NULL;  // so simply clear.
    }
  }
  else
  {
    // no Mutex in JCB to clear
  }
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // }}}  critical section exit  }}}
  return theMutex;
}
//  this usually used for a Thread going to termination and so need to release
//  all resource(Mutes for now) .
int    cleanAllMutexFromHoldingList(R2BTCB_t * theTCB)
{
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // {{{  critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    { //removeMutexFromThreadHoldingList((Mutex_t *)theTCB->baseTCB.pxMutexHeld, theTCB);
      // TODO may optimize later
      Mutex_t * releasingMutex = theTCB->baseTCB.pxMutexHeld;
      while(releasingMutex != NULL)
      {
#if  (SUPPORT_MUTEX_ATTR_AUTORELS)  //  {{{ {{{
        if (osMutexRobust & releasingMutex->mutex_attr)
        { // release the Mutex as MUTEX_ATTR set AUTORELS
          releasingMutex->owner_thread = NULL;
#if  (SUPPORT_MUTEX_ATTR_RECURSIVE) //  {{{ {{{
          releasingMutex->lock = 0;
#endif  //  }}} #if  (SUPPORT_MUTEX_ATTR_RECURSIVE) }}}
          //
          distributeMutex(releasingMutex);
        }
        else
#endif  //  }}} #if  (SUPPORT_MUTEX_ATTR_AUTORELS)  }}}
        {//  pick next Mutex from pxMutexHeld List
          releasingMutex = releasingMutex->xMutexListItem.next;
          // or using pNextMutex ?
          //releasingMutex = releasingMutex->pNextMutex;
        }
      }
    }
    // }}}  critical section exit   }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return 0;
}
