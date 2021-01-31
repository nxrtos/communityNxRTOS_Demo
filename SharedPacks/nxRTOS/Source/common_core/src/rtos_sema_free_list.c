/**
  ******************************************************************************
  * @file           : rtos_sema_free_list.c
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
#include  "rtos_sema_free_list.h"
#include  "rtos_semaphore.h"
#include  "rtos_defer_tcb.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_jcb.h"
#include  "rtos_jcb_ready_list.h"
#include  "rtos_tcb_live_list.h"
#include  "nxRTOSConfig.h"

#if   (RTOS_SYS_SEMAPHORE_NUM)
  static  Sema_t resSysSemaphore[RTOS_SYS_SEMAPHORE_NUM];
#endif
static Sema_t * pFreeSysSemaList = NULL;

/**
  * @brief  Init Semaphore Free List in system
  * @param  None
  * @retval TBD. error code
  */
int  initSysSemFreeList()
{
  int retV = 0;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // critical section enter
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
#if (RTOS_SYS_SEMAPHORE_NUM)
  resSysSemaphore[0].pNextSema = NULL;
  for(int i = RTOS_SYS_SEMAPHORE_NUM -1; i > 0; i--)
  {
    resSysSemaphore[i].pNextSema = &resSysSemaphore[i-1];
  }
  pFreeSysSemaList = &resSysSemaphore[RTOS_SYS_SEMAPHORE_NUM -1];
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
Sema_t * pxInsertSemToFreeList(Sema_t * theSem)
{
  if(theSem != NULL)
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    // place other sanity check here if wanted
    theSem->pNextSema = pFreeSysSemaList;
    pFreeSysSemaList = theSem;
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // critical section exit    }}}
  }
  return theSem;
}

/**
  * @brief  Pick a Semaphore Control Block from SemaFreeList
  * @param  None
  * @retval the pointer to Semaphore Control Block, get from SemaFreeList.
  *         in case of empty SemaFreeList, return NULL
  */
Sema_t * xpickSemFromFreeList()
{
  Sema_t * theSem;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // critical section enter   {{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  theSem = pFreeSysSemaList;
  if(theSem != NULL)
  {
    pFreeSysSemaList = theSem->pNextSema;
    theSem->pNextSema = NULL;
  }
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // critical section exit    }}}
  return theSem;
}
