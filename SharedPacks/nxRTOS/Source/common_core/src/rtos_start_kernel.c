/* rtos_start_kernel.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */

#include  "rtos_start_kernel.h"
#include  "rtos_tick_process.h"
#include  "list_jcb.h"
#include  "rtos_kernel_state.h"
#include  "rtos_commit_job.h"
#include  "rtos_softtimer.h"
#include  "arch4rtos_prekernel_init.h"
#include  "arch4rtos_firstjob.h"

/// {{{ rtos_start_kernel {{{
/// refer to  osStatus osKernelStart (void)
//   take sys_init_jcb  and  prepare tcb for it.
//   prepare stack for the first Thread and run init_job handle in that context.
//  refer to  BaseType_t xPortStartScheduler( void )

int  rtos_start_kernel()
{
  /// {{{
  arch4rtos_iRaiseSysCriticalLevel(HIGHEST_SYSCRITICALLEVEL);

  arch4rtos_prekernel_init();

  initFreeSoftTimerList();
  initSysSemFreeList();
  initSysMutexFreeList();

  xSetRtosState(Kernel_Running);
  arch4rtos_iDropSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  //  }}}
  {
    JCB_t * pTheJCB;

    pTheJCB = xInitListJCB();
    if(NULL == pTheJCB)
    {
      while(1); // un-expected
    }

    pTheJCB = xpickFreeListJCB();
    if(NULL == pTheJCB)
    {
      while(1); // un-expected
    }
    else
    {
      // prepare  initialJCB
      pTheJCB->iJobPar = 0;
      pTheJCB->pJobData = NULL;
      pTheJCB->pThreadStack= ThreadStackTempStacking;
#ifdef  RTOS_INIT_THREAD_STACKSIZE
      pTheJCB->stackSize = RTOS_INIT_THREAD_STACKSIZE;
#else
      pTheJCB->stackSize = 0x00; // RTOS_RUN2BLCK_STACKSIZE;
#endif
      pTheJCB->stateOfJcb = JCB_STATE_READY;
      pTheJCB->threadEntryFunc = initial_Job_Handler;
      pTheJCB->uxPriority = IrqPriorityMaskLevelNOMASK;
      pTheJCB->xJcbListItem.next = NULL;
      pTheJCB->xJcbListItem.prev = NULL;
      pTheJCB->actOption = RelocateToThreadStack;

      // ?? should push theJCB into readyList??
      // addReadyListJCB(pTheJCB);

      // need this for CM0 using softBASEPRI, may optimize later
      // if NO HW BASEPRI
      while(THREAD_SYSCRITICALLEVEL !=
                    arch4rtos_iDropSysCriticalLevel(THREAD_SYSCRITICALLEVEL));
      //rtosSetRtosTickEnable(1);     // enable rtosTick
      arch4rtos_firstjob(pTheJCB);
    }
  }
  return  0;
}

// app must provide customized version
__weak int initial_Job_Handler(void * par)
{
  do
  {
    idle_Job_Handler(par);
  }while(0);

  return 0;
}

// app must provide customized version
__weak int idle_Job_Handler(void * par)
{
  // customized idle processing go here

  {   // sanity check
    SysCriticalLevel_t criticalLevel = arch4rtos_iGetSysCriticalLevel();
    if(criticalLevel != THREAD_SYSCRITICALLEVEL)
    {
      do
      {
        arch4rtos_iDropSysCriticalLevel(THREAD_SYSCRITICALLEVEL);
      }while(0);
    }
  }
  // self driven model
  rtos_commit_job(NULL,
                  0 ,   // job_par
                  NULL,  ThreadStackTempStacking,
                  RTOS_IDLE_THREAD_STACKSIZE /*iStackSize*/,
                  //0,
                  idle_Job_Handler /*pJobHandler*/,
                  RTOS_LOWEST_THREAD_PRIORITY /*xJobPriority*/,
                  RelocateToThreadStack);
  return 0;
}

/// }}} rtos_start_kernel }}}
