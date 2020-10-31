/* rtos_unified_jobhandler.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * 1 tab == 4 spaces!
 */

#include  "rtos_unified_jobhandler.h"
#include  "rtos_mutex.h"
#include  "rtos_jcb.h"
#include  "list_jcb.h"
#include  "list_tcb.h"

int unified_Job_Handler(void * par)
{
  int	ret = -1;
  JCB_t * theJCB = (JCB_t *)par;
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

  if(pxCurrentTCB->pxOwnedObjList != NULL)
  { // the currentTCB has owned Mutex
    cleanAllMutexFromHoldingList((void *)pxCurrentTCB);
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
  arch4rtos_thread_termination(pxCurrentTCB);
  // no return
  return ret;
}
