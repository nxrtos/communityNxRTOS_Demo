/* rtos_xcb_base.h
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */

/*-----------------------------------------------------------------------------
* define structure base part for JobControlBlock and ThreadControlBlock
 *---------------------------------------------------------------------------*/

#ifndef RTOS_XCB_BASE_H    ///  {{{
#define RTOS_XCB_BASE_H    ///  {{{

#if 0
#include  "rtos_xcb_base.txt.h"
#endif

#include  "arch4rtos.h"
#include  "list.h"

typedef  enum   _xcb_state_enum
{
    JCB_FREE,
    JCB_WAITINGSMPLDEFER, // wait a SimplyDefer
    JCB_WAITINGSFTTIMER,   // wait a SoftTimer
    JCB_WAITINGSEMAG,   // wait a Semaphore
    JCB_WAITINGMUTEX,   // wait a Mutex
//    JCB_READY,    // ready to run
//    JCB_RUN,      // running
//    JCB_DEL,      // termination

    JCB_STATE_READY,                // ready to run
    JCB_STATE_READY_WITHSMPLDEFER,  // got a SimplyDefer
    JCB_STATE_READY_WITHSFTTIMER,   // got SftTimer ready to run
    JCB_STATE_READY_WITHSEMA,       // got Sema ready to run
    JCB_STATE_READY_WITHMUTEX,      // got Mutex ready to run

    JCB_STATE_RUN,      // running
    JCB_STATE_RUNWITHSFTTIMER,  // running with holding SftTimer
    JCB_STATE_RUN_WITHMUTEX,    // running with holding Mutex

    JCB_STATE_DEL,      // termination
    JCB_STATE_END,

    // LongLive Thread
    TCB_RUNNING,
    TCB_WAITINGSEMA,    // may with timer, check separated pointer to softTimer
    TCB_WAITINGMUTEX,   // may with timer, check separated pointer to softTimer
    TCB_DEFERRING,      // simply wait timer
    TCB_TERMINATING,    //
    // ShortLive Thread
    TCB_RUN2TERM,
} XCB_State_t;

typedef  enum   _mutex_state_enum
{
  MUTEX_FREE,
  MUTEX_AVAILABLE,
  MUTEX_ASSIGNED,
  MUTEX_LOCKED,
  MUTEX_END,
} Mutex_State_t;

typedef struct thread_control_block_t  TCB_t;

typedef struct _jcb_list_st
{
  struct  job_control_t   * next, * prev;
}ListJCBItem_t;

struct  thread_control_block_t;

typedef struct _basetcb_list_st
{
  struct  thread_control_block_base_t   * next, * prev;
}ListBTCBItem_t;

typedef struct _tcb_list_st
{
  struct  thread_control_block_t   * next, * prev;
}ListTCBItem_t;

struct  _mutex_struct;
typedef struct _mutex_list_st
{
  struct  _mutex_struct   * next, * prev;
}ListMutexItem_t;

typedef enum stack_autoalloc_type_enum
{  // special number chosen never to be valid address for stack pointer
   StackLongPreserving = (-1),    // for Longlive_Thread
   StackShortPreserving = 1L      // for Shortlive_Thread
} StackAllocType_t;

struct  _semaphore_struct;
struct  _mutex_struct;

#define     Xcb_Base_Items                  \
    union                                   \
    {                                       \
        StackType_t *   pxTopOfStack;       \
        StackType_t *   pThreadStack;       \
        TCB_t       *   pTCB;   /*for JCB held running Job*/\
        StackAllocType_t  stackspace_type;  \
        /* for JCB ask for auto allocation of stackspace */ \
        struct  _semaphore_struct * pNextSema;    /* point to next Sema in freeSemList*/\
        struct  _mutex_struct * pNextMutex;  \
    };                                      \
    union                                   \
    {                                       \
        ListXItem_t   baseList;             \
        ListBTCBItem_t xBTcbListItem;       \
        ListJCBItem_t xJcbListItem;         \
        ListTCBItem_t xTcbListItem;         \
        ListMutexItem_t xMutexListItem;     \
    };                                      \
    union                                   \
    {                                       \
        XCB_State_t   state_XCB;    /* state to cover JCB_state and TCB_state*/\
        XCB_State_t   stateOfJcb;   /* state to cover JCB_state */ \
        XCB_State_t   stateOfTcb;   /* state to cover TCB_state */ \
        Mutex_State_t stateOfMutex; /* state to cover TCB_state */ \
    }

typedef struct job_thread_control_block_t
{
    Xcb_Base_Items;
} XCB_Base_t;

#endif	// }}} RTOS_XCB_BASE_H }}}
