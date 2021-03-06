/**
  ******************************************************************************
  * @file           : rtos_commit_mutex_job.c
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
#include  "rtos_commit_job.h"
#include  "rtos_mutex.h"
#include  "rtos_mutex_free_list.h"
#include  "rtos_mutex_waiting_list.h"
#include  "rtos_mutex_holding_list.h"
#include  "rtos_jcb.h"
#include  "rtos_jcb_free_list.h"
#include  "rtos_jcb_ready_list.h"
#include  "rtos_tcb_live_list.h"
#include  "arch4rtos_criticallevel.h"
#include  "nxRTOSConfig.h"
#if  (SUPPORT_MUTEX_CONDITION_JOB) // {{{
/// {{{ rtos_commit_mutexjob    {{{
JCB_t * rtos_commit_mutexjob(JCB_t * pxJCB, int iJobPar, void * pxJobData,
                        StackType_t * pThreadStack, int iStackSize,
                        JobHandlerEntry * pJobHandler, uint32_t xJobPriority,
                        JCB_ActOption_t autoAct, Mutex_t * pMutex4Job)
{
  // "community version does NOT support MUTEX_CONDITION_JOB"
  return NULL;
}
/// }}} rtos_commit_Mutexjob }}}
#endif  // when support mutexjob  }}} }}}
