/* rtos_tcb.h
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */


/*-----------------------------------------------------------------------------
 * define thread control block
 *---------------------------------------------------------------------------*/

#ifndef THREAD_CONTROL_BLOCK_H
#define THREAD_CONTROL_BLOCK_H

#include  "rtos_xcb_base.h"
#include  "arch4rtos.h"
#include  "list.h"
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

typedef enum      _tcb_runtime_type_
{
    Run2Term_TCB_Type,
    Run2Blck_TCB_Type,

    TCB_RunTimeType_End
}   TCB_RunTimeType_t;

typedef enum      _wait_flag_type_
{
    Wait4Ticks     = 1,
    Wait4Sem     = 1 << 1,
    Wait4Mutex     = 1 << 2,
    Wait4MultiMore     = 1 << 3,
}WaitingFlag_t;

typedef enum      _runtime_tcb_extype
{
    R2Term_TCB,
    R2Blck_TCB,
}TCB_eType_t;


typedef struct thread_control_block_t
{
    Xcb_Base_Items; // must be in head
    struct job_control_t *  pxJCB;          /*< Points to the Job Control Block
                                             * which start the thread */
    UBaseType_t uxPriority;                 /*< The priority of the thread.
                                            * consider to have a user defined
                                            * TPriorityType_t for thread
                                            * priority type.
                                            * User defined the lowest priority.
                                            * RTOS_LOWEST_THREAD_PRIORITY
                                            * RTOS_TERMINATION_THREAD_PRIORITY*/
    ListXItem_t xStateListItem;             /*< The list that the state list
                                            * item of a task is reference from
                                            * denotes the state of that task
                                            * (Ready, Blocked, Suspended ).*/
    // TCB_RunTimeType_t        runtime_type;   // ??
    #if  (RTOS_THREAD_STACK_OVERFLOW_TRACE == 1)   ///{{{{
    StackType_t *   pxStackMarkPosition;
    #endif  /// }} (RTOS_THREAD_STACK_OVERFLOW_TRACE == 1) }}

    #if ( RTOS_SUPPORT_MUTEX == 1 )
    UBaseType_t             uxOrigPriority;     /*< The priority before Resource Owner Promotion .*/
	void    *                       pxOwnedObjList;     // point to list Obj Owned by Thread
    #endif

    TCB_eType_t     extTCBType;     // indicate the extension type of TCB
    #if ( configUSE_TRACE_FACILITY == 1 )
    UBaseType_t     uxTCBNumber;        /*< Stores a number that increments each time a TCB is created.  It allows debuggers to determine when a task has been deleted and then recreated. */
    UBaseType_t     uxTaskNumber;       /*< Stores a number specifically for use by third party trace code. */
    //  StackType_t             *pxStack;           /*< Points to the start of the stack. */ /*MOVED to JCB */
    //  char                    pcTaskName[ configMAX_TASK_NAME_LEN ];/*< Descriptive name given to the task when created.  Facilitates debugging only. */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                                                    // MOVED to JCB if needed
    #endif
} BaseTCB_t;	// the base Thread_Control_Block

typedef struct run2termination_thread_control_block_t
{
    BaseTCB_t   baseTCB;
#if ( RTOS_SUPPORT_DELAY_IN_THREAD == 1 )
    TickType_t      xResumeOnTicks;
#endif

    ListXItem_t     xEventListItem;    /*< Used to reference a task from
                                            * an event list. */
    void *          pxWaitingObj;
    WaitingFlag_t   wFlags;

}   R2TTCB_t;   // run2termination  Thread_Control_Block

typedef struct run2blocking_thread_control_block_t
{
    BaseTCB_t   baseTCB;


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
}   R2BTCB_t;   // run2blocking  Thread_Control_Block

#if ( RTOS_THREAD_STACK_OVERFLOW_TRACE == 1 )
#define  RTOS_STACK_OVERFLOW_TRACE_DEFAULTMARK   ((StackType_t)0xefa55afe)
#endif

#endif	// THREAD_CONTROL_BLOCK_H
