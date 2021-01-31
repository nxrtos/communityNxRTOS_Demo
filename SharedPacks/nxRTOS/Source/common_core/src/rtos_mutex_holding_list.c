/**
  ******************************************************************************
  * @file           : rtos_mutex_holding_list.c
  * @brief          : To each liveThread, there is a Mutex hodling list keep
  *                   records of Mutexes currently owned by the Thread.
  *                   Each element in the list is a Mutex Control Block, and
  *                   a build-in List within Mutex_t helps to maintain the List.
  * ****************************************************************************
  * @attention
  *                 nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */

/*-----------------------------------------------------------------------------
 * Mutex control block
 *---------------------------------------------------------------------------*/
#include  "rtos_mutex_holding_list.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_jcb.h"
#include  "rtos_jcb_ready_list.h"

/**
  * @brief  tell if a Mutex is in a liveTCB's MutexHeld List.
  * @param  theMutex, pointer to the Mutex Control Block, to which test if it
  *         is theLiveTCB's MutexHeld List.
  *         theLiveTCB, pointer to liveThread Control Block,
  * @retval 0, does not find theMutex in theLiveTCB's MutexHeld List.
  *         1, find theMutex in theLiveTCB's MutexHeld List.
  */
int  isMutexInHeldBinding(Mutex_t * theMutex, LiveTCB_t * theLiveTCB)
{
  Mutex_t * refMutex = NULL;
  if((NULL != theLiveTCB) && (NULL != theLiveTCB->pxMutexHeld))
  {
    refMutex = theLiveTCB->pxMutexHeld;
    do
    {
      if(refMutex == theMutex)
      {
        break;
      }
      else
      {
        refMutex = refMutex->xMutexListItem.next;
      }
    }while(refMutex != NULL);
  }
  return (refMutex == theMutex);
}
/**
  * @brief  Add a Mutex into liveTCB's MutexHeld List.
  *         This function is invoked when binding a Mutex with liveTCB.
  *         It is a utility function used internally by the kernel, and
  *         presumably with RTOS_SYSCRITICALLEVEL protected, so it may
  *         optimized to skip parameter checking and RTOS_SYSCRITICALLEVEL
  *         protection.
  * @param  theMutex, pointer to the Mutex Control Block, to which need to
  *         add to  liveTCB's MutexHeld List.
  * @retval error code or theMutex
  */
Mutex_t * pxAddMutexToHeldBinding(Mutex_t * theMutex, LiveTCB_t * theLiveTCB)
{
  if((theMutex != NULL) && (theLiveTCB != NULL))
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

    if(theMutex->owner_thread == NULL)
    {
      theMutex->owner_thread = theLiveTCB;
    }
    if(theMutex->owner_thread == theLiveTCB)
    { // put theMutex to be new head
      if(theLiveTCB->pxMutexHeld != NULL)
      {
        Mutex_t * refMutex = theLiveTCB->pxMutexHeld;
        refMutex->xMutexListItem.prev = theMutex;
      }
      theMutex->xMutexListItem.next = theLiveTCB->pxMutexHeld;
      theMutex->xMutexListItem.prev = NULL;
      theLiveTCB->pxMutexHeld = theMutex;
    }
    else
    { // something wrong
      while(1);
    }
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // critical section exit    }}}
  }
  return theMutex;
}

Mutex_t * pxRemoveMutexFromHeldBinding(Mutex_t * theMutex)
{
  if(theMutex != NULL)
  {
    LiveTCB_t * theLiveTCB;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    theLiveTCB = theMutex->owner_thread;
    if(NULL != theLiveTCB)
    { // theMutex should be in theLiveTCB List.
      // sanity check if wanted
      Mutex_t * refMutex = theMutex->xMutexListItem.next;
      if(refMutex != NULL)
      {
        refMutex->xMutexListItem.prev = theMutex->xMutexListItem.prev;
      }
      refMutex = theMutex->xMutexListItem.prev;
      if(refMutex != NULL)
      {
        refMutex->xMutexListItem.next = theMutex->xMutexListItem.next;
      }
      if(theLiveTCB->pxMutexHeld == theMutex)
      { // update theLiveTCB->pxMutexHeld if theMutex is the head
        theLiveTCB->pxMutexHeld = theMutex->xMutexListItem.next;
      }
    }
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // critical section exit    }}}
  }
  else
  {
    // invalid parameter, catch error or skip
  }
  return theMutex;
}

Mutex_t * pickHeadMutexFromBinding(LiveTCB_t * theLiveTCB)
{
  Mutex_t * theMutex = NULL;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // critical section enter   {{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  if (NULL != theLiveTCB)
  {
    theMutex = theLiveTCB->pxMutexHeld;
    if(theMutex != NULL)
    {
      theLiveTCB->pxMutexHeld = theMutex->xMutexListItem.next;
      if(theLiveTCB->pxMutexHeld != NULL)
      { // clear the Head's ->xMutexListItem.prev, should be theMutex
        theMutex->xMutexListItem.next->xMutexListItem.prev = NULL;
      }
      if(theMutex->owner_thread == theLiveTCB)
      { // clear theMutex ownership, should be theLiveTCB
        theMutex->owner_thread = NULL;
      }
      // clear theMutex 's xMutexListItem.
      theMutex->xMutexListItem.next = NULL;
      theMutex->xMutexListItem.prev = NULL;
    }
    else
    { // nothing to pick
    }
  }
  else
  {
    // invalid parameter
  }
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // critical section exit    }}}
  return theMutex;
}
