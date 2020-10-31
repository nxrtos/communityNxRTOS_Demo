/** rtos_softtimer.h
 *
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */

/*-----------------------------------------------------------------------------
 * SoftTimer
 *---------------------------------------------------------------------------*/

#ifndef  RTOS_SOFTTIMER_H    // {{{
#define  RTOS_SOFTTIMER_H    // {{{

#if  0
#include  "rtos_softtimer.txt.h"
#endif

#include  "arch_basetype.h"
#include  "list.h"
#include  "rtos_jcb.h"

typedef     enum    soft_timer_act_option_e
{
    AutoFree,       // put back to system freeSoftTimerList
    KeptAfterX,     // just leave it do nothing
    AutoReload      // reload the Timer
} SoftTimerActOption_t;

typedef     enum    soft_timer_state_e
{
    Free_State,
    Stop_State,
    Run_State,
    Run_4_Job,      // ticking for start Job
    Run_4_Thread    // ticking for resume LongLive_Thread
}SoftTimerState_t;

struct  _soft_timer_struct;
typedef struct  _soft_timer_struct  SoftTimer_struct;
typedef struct  _softtimer_list
{
  SoftTimer_struct *next, *prev;
}ListSoftTimerItem_t;


typedef struct  _soft_timer_struct
{   // {{{ moving to Xcb_Base_Items {{{
    void  *                 pActionObj;     // could be pointer to JCB, TCB
    ListSoftTimerItem_t     baseXList;      //
    ////// }}}                      }}}
    SoftTimerState_t    tmState;
    //ListXItem_t *       pXItem;     /// this pointer to Item in freeTimerList
                                    ///  or in deferringJCBList JCB_t
    TickType_t          xTimerPeriodInTicks;    // the TimerPeriod,
                                                // for action of reload
    SoftTimerActOption_t    actOption;
    TickType_t          xActionOnTicks;     // for RTOS Kernel use to set the
                                            // point of  OsTick  to Action
} SoftTimer_t;

#ifdef __cplusplus
extern "C" {
#endif

int     initFreeSoftTimerList(void);
SoftTimer_t     * xAcquireFreeSoftTimer(void);
int xReleaseSoftTimer(SoftTimer_t * theSoftTimer);
SoftTimer_t     * xCreateSoftTimer(SoftTimer_t * theSoftTimer,
                                   SoftTimerActOption_t  theActOption,
                                   TickType_t      xPeriodInTicks,
                                   JCB_t * pTimedJCB);
/// TODO  remove xCreateSoftTimer and leave pxSetupSoftTimer only ???
SoftTimer_t *   pxSetupSoftTimer(SoftTimer_t * theSoftTimer,
                                   SoftTimerActOption_t  theActOption,
                                   TickType_t      xPeriodInTicks,
                                   JCB_t * pTimedJCB,
                                   int iJobPar,
                                   void * pxJobData,
                                   StackType_t    * pThreadStack,
                                   int iStackSize,
                                   JobHandlerEntry    * pJobHandler,
                                   uint32_t xJobPriority,
                                   JCB_ActOption_t autoAct     );

SoftTimer_t *   xStartSoftTimer(SoftTimer_t * theSoftTimer );
SoftTimer_t *   xStopSoftTimer(SoftTimer_t * theSoftTimer );

#ifdef __cplusplus
}
#endif

#endif	//  }}} RTOS_SOFTTIMER_H }}}
