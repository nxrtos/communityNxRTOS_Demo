/* rtos_stackspace_shortlive_thread.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */

#include  "rtos_stackspace.h"
#include  "new_stackspace4thread.h"
#include  "arch4rtos_criticallevel.h"
#include  "arch4rtos_threadstack.h"
#include  "rtos_tcb_live_list.h"
#include  "nxRTOSConfig.h"

/// {{{     {{{
#if     defined(RTOS_INITIAL_RUN2TERM_ThreadSP)
//const   StackType_t  *  INITIAL_Run2TermThread_StackPoint  =
//                                              RTOS_INITIAL_RUN2TERM_ThreadSP;
#else
#if     (RTOS_Run2TermThreadStack_SIZE)
static StackType_t      theRun2TermThread_StackSPACE
                                              [RTOS_Run2TermThreadStack_SIZE];

#if    defined(STACK_GROW_TOLOWADDR)
const   StackType_t  *  INITIAL_Run2TermThread_StackPoint  =
                &theRun2TermThread_StackSPACE[RTOS_Run2TermThreadStack_SIZE -1];
#else
const   StackType_t  *  INITIAL_Run2TermThread_StackPoint  =
                                        &theRun2TermThread_StackSPACE[0];
#endif

#else
#error  "Can't decide  INITIAL_Run2TermThread_StackPoint "
#endif
#endif
/// }}}     }}}

// get a RAM chunk for stack of a run_to_termination_thread
//

StackType_t * newStackSpace4Run2TermThread(unsigned long  size)
{
  StackType_t * theSP = NULL;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

  // critical section enter
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

  if(getCurrentRun2TermTCB() == NULL)
  { // this will be first run2TermTCB
    //theSP =(StackType_t *) INITIAL_Run2TermThread_StackPoint;
    theSP = getInitialSPSetting4Run2TermThread();
  }
  else
  {
    theSP = NULL;
  }
  // critical section exit
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  return  theSP;
}
