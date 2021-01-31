/**
  ******************************************************************************
  * @file           : rtos_mutex_free_list.h
  * @brief          :
  * ****************************************************************************
  * @attention
  *                 nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */

/*-----------------------------------------------------------------------------
 * Mutex control block freeList
 *---------------------------------------------------------------------------*/
// {{{
#ifndef RTOS_MUTEX_FREE_LIST_H
#define RTOS_MUTEX_FREE_LIST_H
#if  0
#include  "rtos_mutex_free_list.txt.h"
#endif
#include  "rtos_jcb.h"
#include  "rtos_mutex_base.h"
#ifdef __cplusplus
extern "C" {
#endif

int  initSysMutexFreeList(void);
Mutex_t * pxInsertMutexToFreeList(Mutex_t * theMutex);
Mutex_t * xpickMutexFromFreeList(void);

#ifdef __cplusplus
}
#endif

#endif	// RTOS_MUTEX_FREE_LIST_H }}}
