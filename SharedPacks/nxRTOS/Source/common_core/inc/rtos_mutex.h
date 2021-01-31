/**
  ******************************************************************************
  * @file           : rtos_mutex.h
  * @brief          : mutex API in nxRTOS
  ******************************************************************************
  * @attention
  * nxRTOS Kernel V0.0.1
  * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
  *
  * 1 tab == 2 spaces!
  */
#ifndef RTOS_MUTEX_H  // {{{
#define RTOS_MUTEX_H  // {{{
#if  0
#include  "rtos_mutex.txt.h"
#endif
#include  "rtos_mutex_base.h"
#include  "rtos_mutex_free_list.h"
#include  "rtos_jcb.h"
#include  "rtos_tcb_live.h"

#ifdef __cplusplus
extern "C" {
#endif
#if  (SUPPORT_MUTEX_ATTRIBUTES)
Mutex_t * pxMutexNew(Mutex_t *, osMutexAttr_t *);
#else
Mutex_t * pxMutexNew(Mutex_t *);
#endif
int   xMutexDelete(Mutex_t * theMuex);
int   xMutexRelease( Mutex_t * theMuex);
BaseTCB_t * xMutexGetOwner(Mutex_t * theMuex);


void *       xMutexGetWaiter(Mutex_t * theMuex);
Mutex_t *   pxThreadMutexAcquire(Mutex_t * theMuex, TickType_t timeout);
Mutex_t *   pxJobMutexAcquire(Mutex_t * theMuex, JCB_t * theJCB);

int    pickTCBFromMutexWaitingList(Mutex_t * theMutex, R2BTCB_t * theTCB);
int    cleanAllMutexFromHoldingList(R2BTCB_t * theTCB);

int    addJCBToMutexWaitingList(Mutex_t * theMutex, JCB_t * theJCB);
int    pickJCBFromMutexWaitingList(Mutex_t * theMutex, JCB_t * theJCB);

Mutex_t * getMutexInReadyJCB(JCB_t * theJCB);

int distributeMutex(Mutex_t * theMutex);
#ifdef __cplusplus
}
#endif

#endif  // }}} RTOS_MUTEX_H }}}
