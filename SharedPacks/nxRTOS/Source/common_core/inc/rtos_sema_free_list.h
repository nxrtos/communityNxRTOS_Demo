/**
  ******************************************************************************
  * @file           : rtos_sema_free_list.h
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
// {{{
#ifndef RTOS_SEMA_FREE_LIST_H
#define RTOS_SEMA_FREE_LIST_H
#if  0
#include  "rtos_sema_free_list.txt.h"
#endif
#include  "rtos_jcb.h"
#include  "rtos_sema_base.h"
#ifdef __cplusplus
extern "C" {
#endif

int  initSysSemFreeList(void);
Sema_t * pxInsertSemToFreeList(Sema_t * theSem);
Sema_t * xpickSemFromFreeList(void);

#ifdef __cplusplus
}
#endif

#endif	// RTOS_SEMA_FREE_LIST_H }}}
