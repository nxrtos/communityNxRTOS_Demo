/**
  ******************************************************************************
  * @file           : rtos_jcb_free_list.h
  * @brief          : freeJCBList operation
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
#ifndef RTOS_JCB_FREE_LIST_H   //  {{{
#define RTOS_JCB_FREE_LIST_H   //  {{{

//#include  "list_jcb_txt.h"
#include  "arch4rtos.h"
#include  "list.h"
#include  "rtos_jcb.h"

#ifdef __cplusplus
extern "C" {
#endif

/// {{{ freeListJCB {{{
JCB_t * xInitfreeListJCB(void);
JCB_t * xappendFreeListJCB(JCB_t * toFreeList);
JCB_t * xpickFreeListJCB(void);
/// }}} freeListJCB }}}

JCB_t * xInitListJCB(void);
#ifdef __cplusplus
}
#endif

#endif  // }}}  RTOS_JCB_FREE_LIST_H }}}
