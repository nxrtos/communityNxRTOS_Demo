/**
  ******************************************************************************
  * @file           : rtos_mutex_free_list.c
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
#include  "rtos_mutex_free_list.h"
#include  "rtos_defer_tcb.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_jcb.h"
#include  "rtos_jcb_ready_list.h"
#include  "rtos_tcb_live_list.h"
#include  "nxRTOSConfig.h"

#if   (RTOS_SYS_MUTEX_NUM)
  static  Mutex_t resSysMutex[RTOS_SYS_MUTEX_NUM];
#endif
static Mutex_t * pFreeSysMutexList = NULL;

/**
  * @brief  Init Semaphore Free List in system
  * @param  None
  * @retval TBD. error code
  */
int  initSysMutexFreeList()
{
  int retV = 0;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // critical section enter
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
#if (RTOS_SYS_MUTEX_NUM)
  resSysMutex[0].pNextMutex = NULL;
  for(int i = RTOS_SYS_MUTEX_NUM -1; i > 0; i--)
  {
    resSysMutex[i].pNextMutex = &resSysMutex[i-1];
  }
  pFreeSysMutexList = &resSysMutex[RTOS_SYS_MUTEX_NUM -1];
#endif
  // critical section exit
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  return retV;
}
/**
  * @brief  Insert a Semaphore Control Block into SemaFreeList
  * @param  theSem, point to the Semaphore Control Block need to put back into
  *         SemaFreeList
  * @retval TBD. the theSem self or error code, or new pFreeSysSemaList Head?
  */
Mutex_t * pxInsertMutexToFreeList(Mutex_t * theMutex)
{
  if(theMutex != NULL)
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    // place other sanity check here if wanted
    theMutex->pNextMutex = pFreeSysMutexList;
    pFreeSysMutexList = theMutex;
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // critical section exit    }}}
  }
  return theMutex;
}

/**
  * @brief  Pick a Mutex Control Block from MutexFreeList
  * @param  None
  * @retval the pointer to Mutex Control Block, get from MutexFreeList.
  *         in case of empty MutexFreeList, return NULL
  */
Mutex_t * xpickMutexFromFreeList()
{
  Mutex_t * theMutex;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // critical section enter   {{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  theMutex = pFreeSysMutexList;
  if(theMutex != NULL)
  {
    pFreeSysMutexList = theMutex->pNextMutex;
    theMutex->pNextMutex = NULL;
  }
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // critical section exit    }}}
  return theMutex;
}


#if 0
int     initSysMutexFreeList()
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

     // critical section enter
     arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

#if     (RTOS_SYS_MUTEX_NUM)
     resSysMutex[0].pXItem = NULL;
     for(int i = RTOS_SYS_MUTEX_NUM -1; i > 0; i--)
     {
         resSysMutex[i].pXItem = (ListXItem_t *)&resSysMutex[i-1];
     }
     pFreeSysMutexList = &resSysMutex[RTOS_SYS_MUTEX_NUM -1];
#endif

    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return retV;
}
#endif
