/**
  ******************************************************************************
  * @file           : rtos_tcb_base.h
  * @brief          : define Thread Control Block Base type
  ******************************************************************************
  * @attention
  * nxRTOS Kernel V0.0.1
  * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
  *
  * 1 tab == 2 spaces!
  */
/*-----------------------------------------------------------------------------
 * BaseTCB_t shared by JCB and TCB
 *---------------------------------------------------------------------------*/

#ifndef  RTOS_TCB_Base_H  //  {{{
#define  RTOS_TCB_Base_H  //  {{{

#include  "rtos_xcb_base.h"
#include  "arch4rtos.h"
#include  "nxRTOSConfig.h"

// define Tcb_Base_Items  on Xcb_Base_Items
#define     Tcb_Base_Items                  \
            Xcb_Base_Items;                 \
            UBaseType_t uxPriority        /*< The priority of the thread.
                                          * consider to have a user defined
                                          * TPriorityType_t for thread
                                          * priority type.
                                          * User defined the lowest priority.
                                          * RTOS_LOWEST_THREAD_PRIORITY
                                          * RTOS_TERMINATION_THREAD_PRIORITY*/

///*  pre-declare structure of job_control_t *//
struct job_control_t;

typedef struct thread_control_block_base_t
{
  Tcb_Base_Items;
} BaseTCB_t;  // the base Thread_Control_Block,

#endif  // }}}  RTOS_TCB_Base_H  }}}
