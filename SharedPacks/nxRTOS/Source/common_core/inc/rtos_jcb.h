/* rtos_jcb.h
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */

/*-----------------------------------------------------------------------------
* define structure for job control block
 *---------------------------------------------------------------------------*/

#ifndef RTOS_JOB_CONTROL_BLOCK_H    ///  {{{
#define RTOS_JOB_CONTROL_BLOCK_H    ///  {{{

#if 0
#include  "rtos_jcb.txt.h"
#endif

#include  "arch4rtos.h"
#include  "rtos_tcb_base.h"
#include  "list.h"

typedef  enum   jcb_act_option_enum
{
    NoAction,               // do nothing to JCB automatically from Kernel
    FreeAtStartToRun,       // FreeAfter Run In Thread_context.
    FreeAtTermination,      // FreeAfterThreadTerminate, this is seems be a
                            // less to use case
    RelocateToThreadStack,  // make a copy of JCB into ThreadStack_space.
            // release the original JCB, (and send it back to sysFreeJCBList)
            // if the JCB is from sysFreeJCBList originally.
            // This will make pass  the parameter to JobHandler in run-time
    Preserver               // no Sys Auto Action.
                            // This usually used for static JCB.
} JCB_ActOption_t;

#if     0
// moved to XCBState_t
typedef  enum   state_of_jcb_enum
{
    JCB_STATE_FREE,
    JCB_STATE_COMMIT,   // accepted by system/kernel
    JCB_STATE_READY,    // ready to run
    JCB_STATE_RUN,      // running
    JCB_STATE_DEL,      // termination
    JCB_STATE_END
}JCBState_t;
#endif

typedef  int    (JobHandlerEntry)(void *);

typedef  enum   binding_state_enum
{  //
    NOBINDING,
    HOLDING_MUTEX,
    WAIT_SIMPLEDEFER,
    WAIT_SOFTTIMER,
    WAIT_SEMA,
    WAIT_MUTEX,
    END_OF_BINDING
}  BindingState_t;

/// {{{ Job_Control_Block {{{
struct  job_control_t;
typedef  struct  job_control_t *  JobHandle_t;

typedef  struct  job_control_t
{
  Tcb_Base_Items;
//  {{{ items replaced in Tcb_Base_Items, switch for debug {{{
//  StackType_t *   pThreadStack;
                    /* the pointer to RAM block will be used for
                     * the thread's stack space. The value can be
                     * real pointer if pre-allocated RAM or
                     * global/static RAM provided.
                     * Or predefined special meaning, defined in
                     * StackTypeEnum_t, one of value from
                     * {StackLongKept = (-1),    // for Longlive_Thread
                        StackShortStacking  = 1  // for Shortlive_Thread
                       } */
//  ListJCBItem_t   xJcbListItem;
                                    /* the Job_Control_Block must support
                                     * double way of link-list */
//    JCBState_t      stateOfJcb;
                                /* the state of the Job: pending,
                                 * wait-precondition, executing, terminating,
                                 * freed etc. defined as an enum type
                                 * JCBState_t*/
//    uint32_t        uxPriority;   //* the priority for OS kernel to schedule
                                    /* the execution of the job */
                                    /* this priority also to be used for the
                                     * thread in which the job is started
                                     * executing */
                                    /* IrqPriorityMaskLevelNOMASK  is highest
                                     * Thread_Priority */
// }}} items replaced in Tcb_Base_Items, switch for debug }}}
  JobHandlerEntry *threadEntryFunc; // the function of the job_execution,
                                    /* Its input parameter is a point to the
                                     * JCB which carry out the Thread */
  int             stackSize;
  int             iJobPar;    /* a simple parameter to job_handler. In most
                               * case be sufficient.
                               * In case of complicated data needed to
                               * job_handler, use pJobData */
  void    *       pJobData;   /* arbitrary point to the complicated data
                               * structure.Assigned to NULL when no needed */
  JCB_ActOption_t actOption;  //need sys to auto act on somepoint

  //BindingState_t  bdState;  /// The JCB's binding state, may combine
                              /* to stateOfJcb? */
  void    *       pxBindingObj; // point to binding object, could be none,
                                // semaphore, mutex, softTimer etc.*/

} JCB_t;  /// }}} Job_Control_Block }}}


typedef enum stack_type_enum
{ // special number chosen never to be valid address for stack pointer
  StackLongKept = (-1),    // for Longlive_Thread
  StackShortStacking = 1      // for Shortlive_Thread
} StackTypeEnum_t;

#define  ThreadStackTempStacking    ((StackType_t *)StackShortStacking)
#define  ThreadStackLongKeeping     ((StackType_t *)StackLongKept)

#endif  // }}} RTOS_JOB_CONTROL_BLOCK_H }}}
