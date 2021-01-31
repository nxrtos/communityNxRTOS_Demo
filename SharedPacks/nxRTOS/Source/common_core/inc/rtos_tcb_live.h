/**
  ******************************************************************************
  * @file           : rtos_tcb_live.h
  * @brief          : define Thread Control Block
  ******************************************************************************
  * @attention
  * nxRTOS Kernel V0.0.1
  * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
  *
  * 1 tab == 2 spaces!
  */
/*-----------------------------------------------------------------------------
 * define live thread control block structure
 *---------------------------------------------------------------------------*/
#if  0
#include  "rtos_tcb_live.txt.h"
#endif

#ifndef  RTOS_THREAD_CONTROL_BLOCK_H  //  {{{
#define  RTOS_THREAD_CONTROL_BLOCK_H  //  {{{

#include  "rtos_tcb_base.h"
#include  "arch4rtos.h"
#include  "nxRTOSConfig.h"

// refer to tskTCB TCB_t in tasks.c from FreeRTOS
/* The Thread Control Block will be created for each running Thread.
   Here the running Thread means of the context of user space program with
   specific stack location/space assigned.
   The TCB_t shall stay in the bottom of stack, and will and only be removed
   when the thread been killed/terminated.
*/

///*  pre-declare structure of job_control_t *//
struct job_control_t;

typedef enum      _wait_flag_type_
{
    Wait4Ticks     = 1,
    Wait4Sem     = 1 << 1,
    Wait4Mutex     = 1 << 2,
    Wait4MultiMore     = 1 << 3,
}WaitingFlag_t;


#if  (RTOS_THREAD_STACK_OVERFLOW_TRACE == 1)   ///{{{{
  #define  StackMarkPosition_Item           \
                          StackType_t *   pxStackMarkPosition;
#else
#define  StackMarkPosition_Item
#endif  /// }} (RTOS_THREAD_STACK_OVERFLOW_TRACE == 1) }}

#if ( RTOS_SUPPORT_MUTEX == 1 )
  #define  SUPPORT_MUTEX_Item                 \
                  UBaseType_t   uxOrigPriority;           \
                    /* The priority before Resource Owner Promotion .*/ \
                  void    *     pxMutexHeld;              \
                    /* point to list Obj Owned by Thread */
#else
  #define  SUPPORT_MUTEX_Item
#endif

#if ( configUSE_TRACE_FACILITY == 1 ) || 0
#define  TRACE_FACILITY_Item                  \
  UBaseType_t     uxTCBNumber;                \
      /*< Stores a number that increments each time a TCB is created.  */ \
      /* It allows debuggers to determine when a task has been deleted */ \
      /* and then recreated. */               \
  UBaseType_t     uxTaskNumber;               \
      /*< Stores a number specifically for use by third party trace code. */  \
      /*  StackType_t *pxStack;           */  \
      /*< Points to the start of the stack. */ /*MOVED to JCB */  \
      /*  char                    pcTaskName[ configMAX_TASK_NAME_LEN ];  */  \
      /*< Descriptive name given to the task when created.  Facilitates */    \
      /* debugging only. */ /*lint !e971 Unqualified char types are allowed */\
      /*for strings and single characters only. */
#else
  #define  TRACE_FACILITY_Item
#endif


#define     LiveTCB_Items                   \
            Tcb_Base_Items;                 \
            StackMarkPosition_Item          \
            SUPPORT_MUTEX_Item              \
            TRACE_FACILITY_Item             \
            struct job_control_t *  pxJCB

typedef struct thread_control_block_t
{
  LiveTCB_Items;
} LiveTCB_t;	// the base Thread_Control_Block

typedef struct run2termination_thread_control_block_t
{
    LiveTCB_t   baseTCB;
#if ( RTOS_SUPPORT_DELAY_IN_THREAD == 1 )
    TickType_t      xResumeOnTicks;
#endif

    ListXItem_t     xEventListItem;    /*< Used to reference a task from
                                            * an event list. */
    void *          pxWaitingObj;
    //WaitingFlag_t   wFlags;

}   R2TTCB_t;   // run2termination  Thread_Control_Block

typedef struct run2blocking_thread_control_block_t
{
  LiveTCB_t   baseTCB;
#if ( RTOS_SUPPORT_DELAY_IN_THREAD == 1 )
  TickType_t      xResumeOnTicks;
#endif
  ListXItem_t     xEventListItem;     /*< Used to reference a task from
                                          * an event list. */
  void    *       pxWaitingObj;       /* the point to Sema or Mutex to wait,
                                      * or the point to Mutex to Owned.
                                      * // when the TCB is a Mutex Owner, it
                                      * can't be waiting to other Sema or
                                      * Mutex, it can only try to acquire
                                      * by timeout == 0.
                                      */
  WaitingFlag_t   wFlags;
} R2BTCB_t;   // run2blocking  Thread_Control_Block

#if ( RTOS_THREAD_STACK_OVERFLOW_TRACE == 1 )
#define  RTOS_STACK_OVERFLOW_TRACE_DEFAULTMARK   ((StackType_t)0xefa55afe)
#endif

#endif  // }}}  RTOS_THREAD_CONTROL_BLOCK_H  }}}
