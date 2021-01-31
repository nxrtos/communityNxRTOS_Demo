/**
  ******************************************************************************
  * @file           : rtos_mutex_waiting_list.h
  * @brief          :
  * ****************************************************************************
  * @attention
  *                 nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */
#ifndef   __RTOS_MUTEX_WAITING_LIST_H  //  {{{
#define   __RTOS_MUTEX_WAITING_LIST_H  //  {{{

#include  "rtos_mutex.h"
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
extern  int initMutexWaitList(Mutex_t * theMutex);
/**
  * @brief  Insert a JCB or TCB into Semaphore Waiting List.
  *         This function is invoked when a new Semaphore creates.
  * @param  theSem, pointer to the Semaphore Control Block, to which need to
  *         insert the JCB or TCB into Waiting List.
  *         theBaseTCB, pointer to the JCB or TCB which going to join into
  *         Waiting List of Semaphore.
  * @retval TBD. error code
  */
extern  Mutex_t * pxInsertToMutexWaitList(Mutex_t * theMutex, BaseTCB_t * theBaseTCB);
extern  Mutex_t * pxRemoveFromMutexWaitList(Mutex_t * theMutex, LiveTCB_t * theBaseTCB);
extern  BaseTCB_t * pickHeadMutexWaitList(Mutex_t * theMutex);
#ifdef __cplusplus
}
#endif
#endif  //  }}} __RTOS_MUTEX_WAITING_LIST_H  }}}
