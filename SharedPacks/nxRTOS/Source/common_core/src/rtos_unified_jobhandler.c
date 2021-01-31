/* rtos_unified_jobhandler.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  nxrtos or its affiliates.  All Rights Reserved.
 * *
 * 1 tab == 4 spaces!
 */

#include  "rtos_unified_jobhandler.h"
#include  "rtos_mutex.h"
#include  "rtos_jcb.h"
#include  "rtos_jcb_free_list.h"
#include  "rtos_jcb_ready_list.h"
#include  "rtos_tcb_live_list.h"

int unified_Job_Handler(void * par)
{
  int	ret = -1;
  JCB_t * theJCB = (JCB_t *)par;
  LiveTCB_t * theLiveTCB = getCurrentTCB();
  SysCriticalLevel_t criticalLevel = arch4rtos_iGetSysCriticalLevel();

  { // sanity check
    //SysCriticalLevel_t criticalLevel = arch4rtos_iGetSysCriticalLevel();
    if(criticalLevel != THREAD_SYSCRITICALLEVEL)
    { // if got here, it worth to little more investigation
      // as either PendSV or SVC_FF does not set the SysCriticalLevel
      // back before exit ISR.
      do
      {
        arch4rtos_iDropSysCriticalLevel(THREAD_SYSCRITICALLEVEL);
      } // while(0); //
      while(1); // to debug
    }
  }

  if(theJCB != NULL)
  {
    // process JCB_ActOption_t
    if(theJCB->actOption == FreeAtStartToRun)
    {
      xappendFreeListJCB(theJCB);
      theJCB = NULL;
      ret = theJCB->threadEntryFunc(theJCB);
    }
    else if( theJCB->actOption == RelocateToThreadStack)
    {
      JCB_t   relocatedJCB = (* theJCB); // copy theJCB to relocatedJCB
      xappendFreeListJCB(theJCB);
      theJCB = &relocatedJCB;

      { // sanity check
        SysCriticalLevel_t criticalLevel = arch4rtos_iGetSysCriticalLevel();
        if(criticalLevel != THREAD_SYSCRITICALLEVEL)
        { // if got here, it worth to little more investigation
          // as either PendSV or SVC_FF does not set the SysCriticalLevel
          // back before exit ISR.
          do
          {
            arch4rtos_iDropSysCriticalLevel(THREAD_SYSCRITICALLEVEL);
          } // while(0); //
          while(1); // to debug
        }
      }

      ret = theJCB->threadEntryFunc(theJCB);
    }
    else // for everything else
    {
      ret = theJCB->threadEntryFunc(theJCB);
    }
  }
  // process just before termination
  if(theJCB != NULL)
  {
    if(theJCB->actOption == FreeAtTermination)
    {
      xappendFreeListJCB(theJCB);
      theJCB = NULL;
    }
  }

  if(theLiveTCB->pxMutexHeld != NULL)
  { // the currentTCB has owned Mutex
    cleanAllMutexFromHoldingList((void *)theLiveTCB);
  }

  { // sanity check
    SysCriticalLevel_t criticalLevel = arch4rtos_iGetSysCriticalLevel();
    if(criticalLevel != THREAD_SYSCRITICALLEVEL)
    {
      do
      {
        arch4rtos_iDropSysCriticalLevel(THREAD_SYSCRITICALLEVEL);
      } // while(0); //
      while(1); // to debug
    }
  }

  // the JCB mostly gone at this point, do not refer it any more
  arch4rtos_thread_termination(theLiveTCB);
  // no return
  return ret;
}
