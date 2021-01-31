/**
  ******************************************************************************
  * @file           : rtos_mutex_waiting_list.c
  * @brief          :
  * ****************************************************************************
  * @attention
  *                 nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */

/*-----------------------------------------------------------------------------
 * Mutex control block
 *---------------------------------------------------------------------------*/
#include  "rtos_mutex_waiting_list.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_jcb.h"
#include  "rtos_jcb_ready_list.h"
/**
  * @brief  Init Mutex Waiting List within Mutex Control Block
  *         This function is invoked when a new Mutexphore creates.
  * @param  theMutex, pointer to the Mutex Control Block
  * @retval TBD. error code
  */
int initMutexWaitList(Mutex_t * theMutex)
{
  int retV = 0;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // critical section enter
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  if(theMutex != NULL)
  {
    theMutex->pxWaitingBaseTCBList = NULL;
  }
  // critical section exit
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  return retV;
}

/**
  * @brief  Insert a JCB or TCB into Mutexphore Waiting List.
  *         This function is invoked when a new Mutexphore creates.
  * @param  theMutex, pointer to the Mutexphore Control Block, to which need to
  *         insert the JCB or TCB into Waiting List.
  *         theBaseTCB, pointer to the JCB or TCB which going to join into
  *         Waiting List of Mutexphore.
  * @retval TBD. error code
  */
Mutex_t * pxInsertToMutexWaitList(Mutex_t * theMutex, BaseTCB_t * theBaseTCB)
{
  if((theMutex != NULL) && (theBaseTCB != NULL))
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    // place other sanity check here if wanted, make sure theBaseTCB is
    // within pxWaitingBaseTCBList

    if(theMutex->pxWaitingBaseTCBList == NULL)
    {
      theMutex->pxWaitingBaseTCBList = theBaseTCB;
      theBaseTCB->xBTcbListItem.next = NULL;
      theBaseTCB->xBTcbListItem.prev = NULL;
    }
    else
    { // universal method to insert into List against priority order
      pxInsertToTCBList((BaseTCB_t * *)&(theMutex->pxWaitingBaseTCBList), theBaseTCB);
    }
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // critical section exit    }}}
  }
  return theMutex;
}

Mutex_t * pxRemoveFromMutexWaitList(Mutex_t * theMutex, LiveTCB_t * theBaseTCB)
{
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // critical section enter   {{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

  if((theMutex != NULL) && (theBaseTCB != NULL))
  { // place other sanity check here if wanted, make sure theBaseTCB is
    // within pxWaitingBaseTCBList
    pxRemoveFromTCBList((BaseTCB_t * *)&(theMutex->pxWaitingBaseTCBList),
                                                      (BaseTCB_t *)theBaseTCB);
  }
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // critical section exit    }}}
  return theMutex;
}

BaseTCB_t * pickHeadMutexWaitList(Mutex_t * theMutex)
{
  BaseTCB_t * theBaseTCB = NULL;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // critical section enter   {{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

  if(theMutex != NULL)
  {
    theBaseTCB = theMutex->pxWaitingBaseTCBList;
    if(theBaseTCB != NULL)
    {
      //pxRemoveFromTCBList((BaseTCB_t * *)&(theMutex->pxWaitingBaseTCBList),
      //                                                            theBaseTCB);
      // update new HEAD of WaitingBaseTCBList
      theMutex->pxWaitingBaseTCBList = theBaseTCB->xBTcbListItem.next;
      if(theMutex->pxWaitingBaseTCBList != NULL)
      {
        theMutex->pxWaitingBaseTCBList->xBTcbListItem.prev = NULL;
      }
    }
  }
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // critical section exit    }}}
  return theBaseTCB;
}

//// {{{  pending for removing {{{

//// }}}  pending for removing }}}
