/**
  ******************************************************************************
  * @file           : rtos_sema_tentative_jcb_list.h
  * @brief          :
  * ****************************************************************************
  * @attention
  *                 nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */
#if  0
#include  "rtos_sema_tentative_jcb_list.txt.h"
#endif
/*-----------------------------------------------------------------------------
 * Semaphore control block
 *---------------------------------------------------------------------------*/
#ifndef  RTOS_SEMA_TENTATIVE_JCB_LIST_H  //{{{
#define  RTOS_SEMA_TENTATIVE_JCB_LIST_H  //{{{
#include  "rtos_sema_base.h"
#include  "rtos_jcb.h"
#ifdef __cplusplus
extern "C" {
#endif
int  initSemTentativeList(Sema_t * theSem);
/** @brief  Insert theJCB into TentativeList of theSem
  * @param  theJCB: pointer to the JCB will be tentatively grant the Sema
  * @param  theSem: point to the Semaphore
  * @retval NULL or error Code
  *         or theJCB on success.
  */
JCB_t * pxInsertToSemTentativeList(JCB_t * theJCB, Sema_t * theSem);
/** @brief  Remove theJCB from TentativeList of theSem
  * @param  theJCB: pointer to the JCB in TentativeList of the Sema
  * @param  theSem: point to the Semaphore
  * @retval NULL or error Code
  *         or theJCB on success.
  */
JCB_t * pxRemoveFromSemTentativeList(JCB_t * theJCB, Sema_t * theSem);
#ifdef __cplusplus
}
#endif
#endif  // }}}  RTOS_SEMA_TENTATIVE_JCB_LIST_H }}}
