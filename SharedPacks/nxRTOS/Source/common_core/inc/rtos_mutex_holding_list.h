/**
  ******************************************************************************
  * @file           : rtos_mutex_holding_list.h
  * @brief          :
  * ****************************************************************************
  * @attention
  *                 nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */
#if  0
#include  "rtos_mutex_holding_list.txt.h"
#endif
#ifndef   __RTOS_MUTEX_HOLDING_LIST_H  //  {{{
#define   __RTOS_MUTEX_HOLDING_LIST_H  //  {{{

#include  "rtos_mutex.h"
#include  "rtos_tcb_base.h"
#ifdef __cplusplus
extern "C" {
#endif
extern  int initMutexOwnership(Mutex_t * theMutex);
extern
Mutex_t * pxAddMutexToHeldBinding(Mutex_t * theMutex, LiveTCB_t * theLiveTCB);
extern  Mutex_t * pxRemoveMutexFromHeldBinding(Mutex_t * theMutex);
extern  Mutex_t * pickHeadMutexFromBinding(LiveTCB_t * theTCB);
#ifdef __cplusplus
}
#endif
#endif  //  }}} __RTOS_MUTEX_HOLDING_LIST_H  }}}
