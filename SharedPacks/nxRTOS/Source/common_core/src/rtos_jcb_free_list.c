/**
  ******************************************************************************
  * @file           : rtos_jcb_free_list.c
  * @brief          : define Thread Control Block Base type
  ******************************************************************************
  * @attention
  * nxRTOS Kernel V0.0.1
  * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
  *
  * 1 tab == 2 spaces!
  */

/*-----------------------------------------------------------------------------
 * Implementation of functions for freeJCBList
 *---------------------------------------------------------------------------*/
#include  "rtos_jcb_free_list.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_tick_process.h"
#include  "nxRTOSConfig.h"

#if (RTOS_SYS_JCB_NUM != 0)
  static JCB_t  preallocJCB[RTOS_SYS_JCB_NUM];
#endif

#if (RTOS_JCB_DEBUG_TRACE)
  static int  freeJCB_num_trace = 0;
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
#if (RTOS_JCB_DEBUG_TRACE)
  freeJCB_num_trace = RTOS_SYS_JCB_NUM;
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
    { // if the freeList is empty now
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
    #if (RTOS_JCB_DEBUG_TRACE)
      freeJCB_num_trace++;
    #endif
  }
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // critical section exit    }}}
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
    #if (RTOS_JCB_DEBUG_TRACE)
      freeJCB_num_trace--;
    #endif
  }
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // critical section exit    }}}
  return theJCB;
}
/// }}} freeListJCB }}}

#include  "rtos_jcb_ready_list.h"
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
