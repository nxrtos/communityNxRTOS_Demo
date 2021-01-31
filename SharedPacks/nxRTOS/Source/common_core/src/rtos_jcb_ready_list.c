/**
  ******************************************************************************
  * @file           : rtos_jcb_ready_list.c
  *
  * 1 tab == 2 spaces!
  ******************************************************************************
  */

/*-----------------------------------------------------------------------------
 * Implementation of functions for readyJCBList
 *---------------------------------------------------------------------------*/

#include  "rtos_jcb_ready_list.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_tick_process.h"
#include  "rtos_sema_base.h"
#include  "rtos_semaphore.h"
#include  "nxRTOSConfig.h"


/// {{{ readyJCBList {{{
// readyListJCB holding the list for ready JCBs in priority order
// readyListJCB_Head point to the highest priority of ready JCB
//
// pxRemoveFromReadyJCBList will take the ready JCB from readyListJCB_Head
// and pxInsertToReadyJCBList will insert the ready JCB into readyListJCB according
// the priority order.
//
// readyListJCB will be double link in order to remove a JCB from readyListJCB
// when require. (aka. cancel a Job).
// pxRemoveFromReadyJCBList will be a special case of removeReadyListJCB

static JCB_t * pReadyJCBList_Head = NULL;

static  JCB_t * pReadyListJCB_Head[RTOS_THREAD_PRIORITY_NUM];
static  JCB_t * pReadyListJCB_Tail[RTOS_THREAD_PRIORITY_NUM];

#if (RTOS_JCB_DEBUG_TRACE)
  static int  readyJCB_num_trace = 0;
#endif

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
JCB_t * pxInsertToReadyJCBList(JCB_t * toReadyList)
#if 01
{
  JCB_t * refJCB;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

  // sanity check
  while(toReadyList == pReadyJCBList_Head);
  // critical section enter
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  refJCB = pReadyJCBList_Head;
  do{
    if(refJCB == NULL)
    { // reached the END of ReadyListJCB, add after readyListJCB_Tail
      toReadyList->xJcbListItem.prev = NULL;
      toReadyList->xJcbListItem.next = NULL;
      pReadyJCBList_Head = toReadyList;
      break;
    }
    else if(refJCB->uxPriority > toReadyList->uxPriority)
    { // add toReadyList before refJCB
      toReadyList->xJcbListItem.next = (JCB_t *)refJCB;
      toReadyList->xJcbListItem.prev = refJCB->xJcbListItem.prev;
      refJCB->xJcbListItem.prev = (JCB_t *)toReadyList;

      if(refJCB == pReadyJCBList_Head)
      {
        pReadyJCBList_Head = toReadyList;
      }
      else
      {
        toReadyList->xJcbListItem.prev->xJcbListItem.next = (JCB_t *)
                                                           toReadyList;
      }
      break;
    }
    else  //if(refJCB->uxPriority <= toReadyList->uxPriority)
    if(refJCB->xJcbListItem.next != NULL)
    { // move refJCB to .next
      refJCB = (JCB_t *)refJCB->xJcbListItem.next;
      continue;
    }
    else
    { // add toReadyList after refJCB
      toReadyList->xJcbListItem.next = refJCB->xJcbListItem.next;
      toReadyList->xJcbListItem.prev = refJCB;
      refJCB->xJcbListItem.next = toReadyList;
      break;
    }
  }while(1);
  #if (RTOS_JCB_DEBUG_TRACE)
    readyJCB_num_trace++;
  #endif
  // critical section exit
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  return toReadyList;
}
#else
{
  BaseTCB_t * pRetBaseTCB;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

  // critical section enter
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  pRetBaseTCB = pxInsertToTCBList((BaseTCB_t * *) &pReadyJCBList_Head,
                                              (BaseTCB_t *)toReadyList);
#if (RTOS_JCB_DEBUG_TRACE)
  readyJCB_num_trace ++;
#endif
  // critical section exit
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
return (JCB_t *)pRetBaseTCB;
}
#endif

JCB_t * pxRemoveFromReadyJCBList(JCB_t * theJCB)
#if  01
{
  JCB_t * refJCB;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

  // critical section enter {{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  if(theJCB != NULL)
  {
    refJCB = (JCB_t *)theJCB->xJcbListItem.next;
    if(refJCB != NULL)
    {
      refJCB->xJcbListItem.prev = theJCB->xJcbListItem.prev;
    }
    refJCB = (JCB_t *)theJCB->xJcbListItem.prev;
    if(refJCB == NULL)
    {// the theJCB is current readyListJCB_Head, update it
      pReadyJCBList_Head = (JCB_t *)theJCB->xJcbListItem.next;
    }
    else
    {
      refJCB->xJcbListItem.next = theJCB->xJcbListItem.next;
    }
    #if (RTOS_JCB_DEBUG_TRACE)
      readyJCB_num_trace--;
    #endif
  }
  // critical section exit  }}}
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  return theJCB;
}
#else
{
  //BaseTCB_t * pRetBaseTCB;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // critical section enter
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  if(NULL != theJCB)
  {
    //pRetBaseTCB =
        pxRemoveFromTCBList((BaseTCB_t * *) &pReadyJCBList_Head,
                                                        (BaseTCB_t *)theJCB);
#if (RTOS_JCB_DEBUG_TRACE)
    readyJCB_num_trace --;
#endif
  }
  // critical section exit
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  return (JCB_t *)theJCB;
}
#endif
JCB_t * pickHeadReadyListJCB()
{
  JCB_t * refJCB;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

  // critical section enter
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  // theTCB = readyListJCB_Head;
  refJCB = pxRemoveFromReadyJCBList(pReadyJCBList_Head);
  // critical section exit
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  return refJCB;
}

JCB_t * getReadyJCB(void)
{
  return  pReadyJCBList_Head;
}
/// }}} readyJCBList }}}
