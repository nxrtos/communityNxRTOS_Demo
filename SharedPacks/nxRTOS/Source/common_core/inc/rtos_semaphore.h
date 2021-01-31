/**
  ******************************************************************************
  * @file           : rtos_semaphore.h
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
#ifndef RTOS_SEMAPHORE_H  //  {{{
#define RTOS_SEMAPHORE_H  //  {{{
#if  0
#include  "rtos_semaphore.txt.h"
#endif
#include  "rtos_sema_base.h"
#include  "rtos_jcb.h"
#include  "rtos_tcb_live.h"
#include  "nxRTOSConfig.h"
#ifdef __cplusplus
extern "C" {
#endif

//int initSysSemFreeList(void);
extern
#if  (SUPPORT_SEMA_ATTRIBUTES)
Sema_t * pxSemNew(Sema_t * theSem, uint32_t max_count,
                  uint32_t initial_count, const osSemaAttr_t * pSema_attr);
#else
Sema_t * pxSemNew(Sema_t * theSem, uint32_t max_count, uint32_t initial_count);
#endif
int  xSemDelete(Sema_t * theSem);
int  xSemRelease(Sema_t * theSem);
int  xSemGetCount(Sema_t * theSem);
Sema_t * pxSemAcquire(Sema_t * theSem, TickType_t timeout);
#if   (SUPPORT_SEMA_CONDITION_JOB)
extern
JCB_t * rtos_commit_semjob(JCB_t * pxJCB, int iJobPar, void * pxJobData,
                        StackType_t * pThreadStack, int iStackSize,
                        JobHandlerEntry * pJobHandler, uint32_t xJobPriority,
                        JCB_ActOption_t autoAct, Sema_t *  pSem4Job);
#endif
/**
  * @brief  utility function walk through to deliver Semaphore to JCB or liveTCB
  *         which in the waitingList.
  *         Invoke on the time
  *         sema->token increased or pxTentativeToJob is cleared.
  * @param  theSem, pointer to the Semaphore Control Block.
  * @retval TBD. error code
  */
int deliverSema(Sema_t * theSem);
#ifdef __cplusplus
}
#endif

#endif  // }}}  RTOS_SEMAPHORE_H }}}
