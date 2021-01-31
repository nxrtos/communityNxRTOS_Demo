/**
  ******************************************************************************
  * @file           : sema_demo_job.c
  * @brief          :
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */
#include "sema_demo_job.h"
#include "rtos_semaphore.h"
#include "job_led_blink.h"
#include "bsp.h"
#include  "nxRTOSConfig.h"

#if   (SUPPORT_SEMA_CONDITION_JOB)    /// {{{
                                      /// {{{
Sema_t * pBlibkSema = NULL;
const osSemaAttr_t theSemAttr = osSemaJobReassign;

int blink_sema_job_handler(void * par)
{
  JCB_t * theJCB =(JCB_t *)par;
  // (void)theJCB;
  if(theJCB->iJobPar <= LED_MAX)
  {
    BSP_LED_Toggle(theJCB->iJobPar);
  }

  // sanity check
  if(pBlibkSema == NULL)
  {
    while(1);
  }
  // return to terminate this thread.
  return 0;
}

int sema_giver(void * par)
{
  JCB_t * theJCB =(JCB_t *)par;
  Sema_t * pTheSema = (Sema_t *)theJCB->pJobData;

  if(pTheSema != NULL)
  {
    xSemRelease(pTheSema);
  }
  else
  {
    while(1);
  }
  return 0;
}

int start_sema_demo(void * par)
{
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // {{{  critical section enter {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(pBlibkSema == NULL)
    {
#if  (SUPPORT_SEMA_ATTRIBUTES)
  #if  01
      pBlibkSema = pxSemNew(pBlibkSema, 1, 0, &theSemAttr);
  #else
      pBlibkSema = pxSemNew(pBlibkSema, 1, 0, NULL);
  #endif
#else
    pBlibkSema = pxSemNew(pBlibkSema, 1, 0);
#endif
    }
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}  critical section exit  }}}
  }
  if(pBlibkSema != NULL)
  {
#if 01
    rtos_commit_job(NULL, 0 ,   //iJobPar
                        pBlibkSema,  ThreadStackTempStacking,
                        SEMADEMO_STACK_SIZE /*iStackSize*/,
                        sema_giver /*pJobHandler*/,
                        SEMADEMO_JOB_PRIORITY -1/*xJobPriority*/,
                        RelocateToThreadStack);
    rtos_commit_semjob(NULL, theLED3 ,   //iJobPar  theLED3,
                  NULL,  ThreadStackTempStacking,
                  SEMADEMO_STACK_SIZE /*iStackSize*/,
                  blink_sema_job_handler /*pJobHandler*/,
                  SEMADEMO_JOB_PRIORITY /*xJobPriority*/,
                  RelocateToThreadStack,
                  pBlibkSema);
#endif
#if  01
    rtos_commit_semjob(NULL, theLED4 ,   //iJobPar  theLED3,
                  NULL,  ThreadStackTempStacking,
                  SEMADEMO_STACK_SIZE /*iStackSize*/,
                  blink_sema_job_handler /*pJobHandler*/,
                  SEMADEMO_JOB_PRIORITY + 1/*xJobPriority*/,
                  RelocateToThreadStack,
                  pBlibkSema);
    rtos_commit_deferjob(NULL, 0 ,   //iJobPar
                        pBlibkSema,  ThreadStackTempStacking,
                        SEMADEMO_STACK_SIZE /*iStackSize*/,
                        sema_giver /*pJobHandler*/,
                        SEMADEMO_JOB_PRIORITY /*xJobPriority*/,
                        RelocateToThreadStack,
                        1 * 600 /*ticks to defer*/);
#endif
#if  01
    rtos_commit_semjob(NULL, theLED5 ,   //iJobPar  theLED3,
                  NULL,  ThreadStackTempStacking,
                  SEMADEMO_STACK_SIZE /*iStackSize*/,
                  blink_sema_job_handler /*pJobHandler*/,
                  SEMADEMO_JOB_PRIORITY -1/*xJobPriority*/,
                  RelocateToThreadStack,
                  pBlibkSema);
    rtos_commit_deferjob(NULL, 0 ,   //iJobPar
                        pBlibkSema,  ThreadStackTempStacking,
                        SEMADEMO_STACK_SIZE /*iStackSize*/,
                        sema_giver /*pJobHandler*/,
                        SEMADEMO_JOB_PRIORITY /*xJobPriority*/,
                        RelocateToThreadStack,
                        2 * 600 /*ticks to defer*/);
#endif
  }

  // defer re-arm self
  rtos_commit_deferjob(NULL, 0 ,   //iJobPar
                      pBlibkSema,  ThreadStackTempStacking,
                      SEMADEMO_STACK_SIZE /*iStackSize*/,
                      start_sema_demo /*pJobHandler*/,
                      SEMADEMO_JOB_PRIORITY /*xJobPriority*/,
                      RelocateToThreadStack,
                      5000 /*ticks to defer*/);

  return 0;
}
/// }}}
#else /// {{{
int start_sema_demo(void * par)
{
  return 0;
}
#endif  /// }}}
/// }}}
