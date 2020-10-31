/*  rtos_tick_process.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * *
 * 1 tab == 4 spaces!
 */

// refer to xPortSysTickHandler( void ),  xTaskIncrementTick()
// this is hook function need to be called from SySTickISR.
// on each SysTick increment.

#include  "rtos_tick_process.h"
#include  "rtos_kernel_state.h"
#include  "rtos_defer_tcb.h"
#include  "rtos_semaphore.h"
#include  "rtos_mutex.h"
#include  "rtos_commit_job.h"
#include  "defer_jcb.h"
#include  "list_jcb.h"
#include  "list_tcb.h"
#include  "arch4rtos_criticallevel.h"
#include  "nxRTOSConfig.h"

static int  rtos_tick_process_enabled = 0;
static volatile TickType_t xRtosTickCount = 0;

int  rtosSetRtosTickEnable(int enable)
{
  rtos_tick_process_enabled = enable;
  return enable;
}

int  rtosIfRtosTickEnable()
{
  return rtos_tick_process_enabled;
}

TickType_t rtosGetRtosTick()
{
  return xRtosTickCount;
}

static  void rtosTickProcess_forDeferTCB(TickType_t  theTick)
{
  SysCriticalLevel_t   origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  //{{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  /// {{{ process deferTCB WaitingList  {{{
  if(theTick == 0)
  {
    while(pxDeferTCBCurrentWaitingList != NULL)
    {
      R2BTCB_t * theTCB =pickFromDeferTCBWaitingList((R2BTCB_t *)
                                                 pxDeferTCBCurrentWaitingList);
      if(theTCB != NULL)
      {
        if((theTCB->wFlags & Wait4Sem)  && (theTCB->pxWaitingObj != NULL))
        {   // timeout on acquire Semaphore, remove from  SemaList
          pickTCBFromSemWaitingList(theTCB->pxWaitingObj, theTCB);
        }
        addTCBToRun2BlckTCBList((BaseTCB_t *)theTCB);
        // it may skip this check and do the check altogether at last
        if((pxCurrentTCB != NULL) &&
            (pxCurrentTCB->uxPriority > ((BaseTCB_t *)theTCB)->uxPriority ))
        {
          arch4rtosReqSchedulerService();// req kernel to re-schedule;
        }
      }
      else
      {   // catch error
        while(1);
      }
    }       //
    pxDeferTCBCurrentWaitingList = pxDeferTCBOverflowWaitingList;
    pxDeferTCBOverflowWaitingList = NULL;
  }
  else
  {
    while((pxDeferTCBCurrentWaitingList != NULL) &&
        (pxDeferTCBCurrentWaitingList->xResumeOnTicks <= theTick))
    {
      R2BTCB_t * theTCB = pickFromDeferTCBWaitingList((R2BTCB_t *)
                                                pxDeferTCBCurrentWaitingList);
      if(theTCB != NULL)
      {
        if((theTCB->wFlags & Wait4Sem)  && (theTCB->pxWaitingObj != NULL))
        { // timeout on acquire Semaphore, remove from  SemaList
          pickTCBFromSemWaitingList(theTCB->pxWaitingObj, theTCB);
        }

        if((theTCB->wFlags & Wait4Mutex)  && (theTCB->pxWaitingObj != NULL))
        {   // timeout on acquire Semaphore, remove from  SemaList
          pickTCBFromMutexWaitingList(theTCB->pxWaitingObj, theTCB);
          // TODO need to indicate in theTCB the reason of resume is
          //             timeout rather than acquired Mutex
        }

        addTCBToRun2BlckTCBList((BaseTCB_t *)theTCB);
        // it may skip this check and do the check altogether at last
        if((pxCurrentTCB != NULL) &&
            (pxCurrentTCB->uxPriority > ((BaseTCB_t *)theTCB)->uxPriority ))
        {
          arch4rtosReqSchedulerService();// req kernel to re-schedule;
        }
      }
      else
      {   // catch error
        while(1);
      }
    }
  }
  /// }}} process deferTCB WaitingList  }}}
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // }}}
}

static  TickType_t xThreadTickCount = 0;
volatile uint8_t  pendingThreadHandler = 0;
#define  RTOS_TICKThreadHANDLER_STACK_SIZE      (0x400)

int rtosTickThreadHandler(void * pData)
{
  SysCriticalLevel_t   origCriticalLevel;

  //JCB_t * theJCB = (JCB_t *)pData;

  { // sanity check
    origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    if(origCriticalLevel != THREAD_SYSCRITICALLEVEL)
    {
      do
      {
        arch4rtos_iDropSysCriticalLevel(THREAD_SYSCRITICALLEVEL);
      } // while(0); //
      while(1); // to debug
    }
  }

  //{{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

  // clear the Pending Flag
  pendingThreadHandler = 0;
  while( xThreadTickCount != xRtosTickCount)
  {
    xThreadTickCount++;
    rtosTickProcess_forDeferTCB(xThreadTickCount);
  }

  // }}}
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);

  { // sanity check
    origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    if(origCriticalLevel != THREAD_SYSCRITICALLEVEL)
    {
      do
      {
        arch4rtos_iDropSysCriticalLevel(THREAD_SYSCRITICALLEVEL);
      } // while(0); //
      while(1); // to debug
    }
  }
  return 0;
}

// TODO, this rtosTickHandler can be invoke from ISR or
// an Run2TermJob with Highest Thread_Priority in Thread_context.
void rtosTickHandler( void )
{
  SysCriticalLevel_t   origCriticalLevel;

  if(!rtosIfRtosTickEnable())
  {   // do all processing only when Kernel_Running
    return;
  }

  //{{{
  origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

  xRtosTickCount++;
  //rtosTickProcess_forDeferTCB(xRtosTickCount);
  /// {{{ process deferJCB WaitingList {{{
  if(xRtosTickCount == 0)
  { // move all in pxOsCurrentSoftTimerWaitingList to readyJCB
    // switch  pxOsCurrentSoftTimerWaitingList   and
    // pxOsOverflowSoftTimerWaitingList
    while(pxOsCurrentSoftTimerWaitingList != NULL)
    {
      SoftTimer_t * theSoftTimer = pickFromSoftTimerWaitingList
                              ((SoftTimer_t *)pxOsCurrentSoftTimerWaitingList);
      // this may need to check the
      // TODO if(theSoftTimer->tmState == Run_4_Job),
      // the softTimer may used for resume LongLive_Thread
      JCB_t * theJCB = (JCB_t *) (theSoftTimer->pActionObj);
      if(theJCB != NULL)
      {
        addReadyListJCB(theJCB);
        // it may skip this check and do the check altogether at last
        if((pxCurrentTCB != NULL) &&
                            (pxCurrentTCB->uxPriority > theJCB->uxPriority ))
        {
          arch4rtosReqSchedulerService();// req kernel to re-schedule;
        }
      }
      else
      {   // catch error,
        while(1);
      }
      theSoftTimer->tmState = Stop_State;
      if(theSoftTimer->actOption == AutoFree)
      {
        xReleaseSoftTimer(theSoftTimer);
      }
      else if (AutoReload == theSoftTimer->actOption)
      {
        // for AutoReload SoftTimer, it needs a new JCB to link to
        // failed to xpickFreeListJCB() will be a critical problem
        // TODO anyway to improve???
        JCB_t * theJCB = xpickFreeListJCB();

        if(theJCB != NULL)
        {   /// copy theJCB from theSoftTimer->pActionObj
          (* theJCB) = *((JCB_t *)theSoftTimer->pActionObj);
          // update to new JCB
          theSoftTimer->pActionObj = (ListXItem_t *)theJCB;
          theSoftTimer->tmState = Stop_State;
          /// the state has to be stop in order to xStartSoftTimer
          xStartSoftTimer(theSoftTimer);
        }
      }
    }	    //
    pxOsCurrentSoftTimerWaitingList = pxOsOverflowSoftTimerWaitingList;
    pxOsOverflowSoftTimerWaitingList = NULL;
  }
  else
  {
    while((pxOsCurrentSoftTimerWaitingList != NULL) &&
        (pxOsCurrentSoftTimerWaitingList->xActionOnTicks <= xRtosTickCount))
    {
      SoftTimer_t * theSoftTimer = pickFromSoftTimerWaitingList
                              ((SoftTimer_t *)pxOsCurrentSoftTimerWaitingList);
      // catch error
      if(theSoftTimer->tmState != Stop_State)
      {
        while(1);
      }
      // TODO if(theSoftTimer->tmState == Run_4_Job)
      // the softTimer may used for resume LongLive_Thread
      JCB_t * theJCB = (JCB_t *) (theSoftTimer->pActionObj);
      if(theJCB != NULL)
      {
        addReadyListJCB(theJCB);
        // it may skip this check and do the check altogether at last
        if((pxCurrentTCB != NULL) &&
                            (pxCurrentTCB->uxPriority > theJCB->uxPriority ))
        {
          arch4rtosReqSchedulerService();// req kernel to re-schedule;
        }
      }
      theSoftTimer->tmState = Stop_State;
      if(theSoftTimer->actOption == AutoFree)
      {
        theSoftTimer->pActionObj = NULL;
        xReleaseSoftTimer(theSoftTimer);
      }
      else if (AutoReload == theSoftTimer->actOption)
      { // theJCB is to be copied
        // for AutoReload SoftTimer, it needs a new JCB to link to
        // failed to xpickFreeListJCB() will be a critical problem
        // TODO anyway to improve???
        JCB_t * theJCB = xpickFreeListJCB();

        if(theJCB != NULL)
        { // copy theJCB from theSoftTimer->pXItem
          (* theJCB) = *((JCB_t *)theSoftTimer->pActionObj);
          // update to new JCB
          theSoftTimer->pActionObj = (ListXItem_t *)theJCB;
          theSoftTimer->tmState = Stop_State;
          /// the state has to be stop in order to xStartSoftTimer
          xStartSoftTimer(theSoftTimer);
        }
      }
    }
  }
  /// }}} process deferJCB WaitingList  }}}

  ///  check reschedule
  if((pxCurrentTCB != NULL) &&  (pxReadyListJCB != NULL) &&
                      (pxCurrentTCB->uxPriority > pxReadyListJCB->uxPriority ))
  {
    arch4rtosReqSchedulerService();// req kernel to re-schedule;
  }



#if  01
  // let rtosTickThreadHandler run in Thread_context
  if(!pendingThreadHandler)
  {   // need a trigger for rtosTickThreadHandler
    rtos_commit_job(NULL, 0,
                    NULL, //void * pxJobData,
                    ThreadStackTempStacking, //StackType_t    * pThreadStack,
                    RTOS_TICKThreadHANDLER_STACK_SIZE,
                    //int iStackSize, size for stack of rtosTickThreadHandler
                    // use GNU stack analysis to check
                    rtosTickThreadHandler, // JobHandlerEntry    * pJobHandler,
                    RTOS_HIGHEST_THREAD_PRIORITY, //uint32_t xJobPriority,
                    RelocateToThreadStack// JCB_ActOption_t autoAct);
                    );
    pendingThreadHandler = 1;
  }
#else
  // direct call from ISR
  rtosTickThreadHandler(NULL/*pxJobData*/);
#endif

  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);

  // }}}
  return;
}

