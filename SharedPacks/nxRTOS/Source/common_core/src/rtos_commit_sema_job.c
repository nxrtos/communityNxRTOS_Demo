/**
  ******************************************************************************
  * @file           : rtos_commit_sema_job.c
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
#include  "rtos_commit_job.h"
#include  "rtos_semaphore.h"
#include  "rtos_sema_free_list.h"
#include  "rtos_sema_waiting_list.h"
#include  "rtos_sema_tentative_jcb_list.h"
#include  "rtos_jcb.h"
#include  "rtos_jcb_ready_list.h"
#include  "rtos_jcb_free_list.h"
#include  "rtos_tcb_live_list.h"
#include  "rtos_defer_tcb.h"
#include  "arch4rtos_criticallevel.h"

#if   (SUPPORT_SEMA_CONDITION_JOB)  /// {{{
/// {{{ rtos_commit_semajob    {{{
JCB_t * rtos_commit_semjob(JCB_t * pxJCB, int iJobPar, void * pxJobData,
                        StackType_t * pThreadStack, int iStackSize,
                        JobHandlerEntry * pJobHandler, uint32_t xJobPriority,
                        JCB_ActOption_t autoAct, Sema_t * pSem4Job)
{ // "community version does NOT support SEMA_CONDITION_JOB"
  return NULL;
}
/// }}}  rtos_commit_semjob    }}}
#endif  /// #if   (SUPPORT_SEMA_CONDITION_JOB)  }}}
