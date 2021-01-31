/**
  ******************************************************************************
  * @file           : rtos_sema_waiting_list.h
  * @brief          :
  * ****************************************************************************
  * @attention
  *                 nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */
#ifndef   __RTOS_SEMA_WAITING_LIST_H  //  {{{
#define   __RTOS_SEMA_WAITING_LIST_H  //  {{{

#include  "rtos_semaphore.h"
#include  "rtos_tcb_base.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
  * @brief  Init Semaphore Waiting List within Semaphore Control Block
  *         This function is invoked when a new Semaphore creates.
  * @param  theSem, pointer to the Semaphore Control Block
  * @retval TBD. error code
  */
extern  int initSemWaitList(Sema_t * theSem);
/**
  * @brief  Insert a JCB or TCB into Semaphore Waiting List.
  *         This function is invoked when a new Semaphore creates.
  * @param  theSem, pointer to the Semaphore Control Block, to which need to
  *         insert the JCB or TCB into Waiting List.
  *         theBaseTCB, pointer to the JCB or TCB which going to join into
  *         Waiting List of Semaphore.
  * @retval TBD. error code
  */
extern  Sema_t * pxInsertToSemWaitList(Sema_t * theSem, BaseTCB_t * theBaseTCB);
extern  Sema_t * pxRemoveFromSemWaitList(Sema_t * theSem, BaseTCB_t * theBaseTCB);
extern  BaseTCB_t * pickHeadSemWaitList(Sema_t * theSem);
#ifdef __cplusplus
}
#endif
#endif  //  }}} __RTOS_SEMA_WAITING_LIST_H  }}}
