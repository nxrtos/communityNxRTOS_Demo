/**
  ******************************************************************************
  * @file           : mutex_demo_job.c
  * @brief          :
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */
#include "mutex_demo_job.h"
#include "rtos_mutex.h"
#include "job_led_blink.h"
#include "bsp.h"

#if   (SUPPORT_MUTEX_CONDITION_JOB)  //  {{{
Mutex_t * pBlibkMutex = NULL;

int blink_mutex_job_handler(void * par)
{
  JCB_t * theJCB =(JCB_t *)par;
  // (void)theJCB;
  if(theJCB->iJobPar <= LED_MAX)
  {
    BSP_LED_Toggle(theJCB->iJobPar);
  }

  if(pBlibkMutex != NULL)
  {
    xMutexRelease(pBlibkMutex);
  }
  else
  {
    while(1);
  }
  // return to terminate this thread.
  return 0;
}


int start_mutex_demo(void * par)
{
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // {{{  critical section enter {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(pBlibkMutex == NULL)
    {
  #if  (SUPPORT_MUTEX_ATTR_RECURSIVE) || (SUPPORT_MUTEX_ATTR_PRIO_INHRT) ||  \
       (SUPPORT_MUTEX_ATTR_AUTORELS)
      pBlibkMutex = pxMutexNew(pBlibkMutex, NULL);
  #else
      pBlibkMutex = pxMutexNew(pBlibkMutex);
  #endif
    }
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}  critical section exit  }}}
  }
  if(pBlibkMutex != NULL)
  {
    rtos_commit_mutexjob(NULL, theLED4 ,   //iJobPar                             theLED3,
                  NULL,  ThreadStackTempStacking,
                  MUTEXDEMO_STACK_SIZE /*iStackSize*/,
                  blink_mutex_job_handler /*pJobHandler*/,
                  MUTEXDEMO_JOB_PRIORITY /*xJobPriority*/,
                  RelocateToThreadStack,
                  pBlibkMutex);
#if  01
    rtos_commit_mutexjob(NULL, theLED3 ,   //iJobPar                             theLED3,
                  NULL,  ThreadStackTempStacking,
                  MUTEXDEMO_STACK_SIZE /*iStackSize*/,
                  blink_mutex_job_handler /*pJobHandler*/,
                  MUTEXDEMO_JOB_PRIORITY /*xJobPriority*/,
                  RelocateToThreadStack,
                  pBlibkMutex);
#endif
#if  01
    rtos_commit_mutexjob(NULL, theLED5 ,   //iJobPar                             theLED3,
                  NULL,  ThreadStackTempStacking,
                  MUTEXDEMO_STACK_SIZE /*iStackSize*/,
                  blink_mutex_job_handler /*pJobHandler*/,
                  MUTEXDEMO_JOB_PRIORITY /*xJobPriority*/,
                  RelocateToThreadStack,
                  pBlibkMutex);
#endif
  }

  // defer re-arm self
  rtos_commit_deferjob(NULL, theLED5 ,   //iJobPar                             theLED3,
                      NULL,  ThreadStackTempStacking,
                      MUTEXDEMO_STACK_SIZE /*iStackSize*/,
                      start_mutex_demo /*pJobHandler*/,
                      MUTEXDEMO_JOB_PRIORITY /*xJobPriority*/,
                      RelocateToThreadStack,
                      2000 /*ticks to defer*/);
  return 0;
}
#else //  }}} {{{
int start_mutex_demo(void * par)
{
  return 0;
}
#endif  ///}}}
