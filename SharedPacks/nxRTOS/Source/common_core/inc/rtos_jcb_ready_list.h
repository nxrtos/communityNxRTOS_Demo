/**
  ******************************************************************************
  * @file           : rtos_jcb_ready_list.h
  * @brief          : readyJCBList operation
  ******************************************************************************
  * @attention
  * nxRTOS Kernel V0.0.1
  * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
  *
  * 1 tab == 2 spaces!
  */
/*-----------------------------------------------------------------------------
 * Implementation of functions for job control
 *---------------------------------------------------------------------------*/
#ifndef RTOS_JCB_READY_LIST_H   //  {{{
#define RTOS_JCB_READY_LIST_H   //  {{{

#if  0
#include  "rtos_jcb_ready_list.txt.h"
#endif

#include  "arch4rtos.h"
#include  "list.h"
#include  "rtos_jcb.h"
#include  "rtos_tcb_base_list.h"

#ifdef __cplusplus
extern "C" {
#endif

extern  volatile JCB_t * pxSysDeferListJCB; // = NULL;
extern  volatile JCB_t * pxSysDeferOverflowListJCB; // = NULL;

JCB_t * getReadyJCB(void);
/// {{{ readyListJCB {{{
void    initReadyListJCB(void);
JCB_t * pxInsertToReadyJCBList(JCB_t * toReadyList);
JCB_t * pxRemoveFromReadyJCBList(JCB_t * pickJCB);
JCB_t * pickHeadReadyListJCB(void);
/// }}} readyListJCB }}}

/// {{{ run2TermListJCB {{{
JCB_t * pushRun2TermListJCB(JCB_t * toList);
JCB_t * popRun2TermListJCB(void);
/// }}} run2TermListJCB }}}

JCB_t * addSysDeferListJCB(JCB_t * pAddJCB, TickType_t  xDeferTicks);
JCB_t * pickSysDeferListJCB(JCB_t * theJCB);

#ifdef __cplusplus
}
#endif

#endif  // }}}  RTOS_JCB_READY_LIST_H }}}
