/* rtos_commit_job.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */
/*-----------------------------------------------------------------------------
 * Thread is program running in user space context, starting from a ready JCB
 * been transferring into executing.
 * Each thread comes with a ThreadControlBlock, which is located in the bottom
 * of stack of the thread.
 *
 * currentRunningThreadP, point to the current running thread's TCB, this
 * will be the highest priority of thread among other running threads and
 * the priority also be no less than readyJCBHead's priority.
 *
 * a lower priority of Thread may preempted by a higher priority of running
 * Thread.
 *
 * When a Thread is type of run2TermThread, it will be treated in a special
 * way by scheduler. As all run2TermThreads's stacks will be stacked up to
 * share a chunk of RAM space, with lower priority of thread in the bottom
 * and highest priority of thread on the top. The thread/stack in the bottom
 * is frozen until the threads on the top all went terminated.
 * As result, there is run2TermThreadList_Head point to a run2TermThread with
 * highest priority among other run2TermThreads. The Thread pointed by
 * run2TermThreadList_Head can increase its own priority to highest Thread
 * priority and decrease to the no lower than its immediate preempted
 * run2TermThread.
 *
 * When a Thread is type of run2BlckThread, it will has an exclusive RAM
 * chunk assigned for stack space of run2BlckThread. When run2BlckThread
 * reach a blocking point to be suspended, the TCB will be moved to a List
 * holding by the corresponding RTOS object (aka Semaphore, Mutex, SysTimer
 * etc). If a run2BlckThread is preempted, it will be kept in run2BlckThreadList.
 *
 * currentRunningThreadP will have the same value to one of run2TermThread_Head
 * or run2BlckThread_Head.
 *
 * A special Init_Thread is a type of run2TermThread, started by startRTOS().
 * At the end of Init_Thread, it needs to commit a self re-arm Idle_Job, which
 * is causing Idle_run2TermThread to executed in exclusive lowest priority
 * repeatedly when no other running Thread and/or ready Job available.
 *
 *---------------------------------------------------------------------------*/

#include  "rtos_commit_job.h"
#include  "rtos_kernel_servicecheck.h"
#include  "list_jcb.h"
#include  "list_tcb.h"
#include  "arch4rtos_criticallevel.h"

/// {{{ rtos_commit_simplejob {{{
///
JCB_t * rtos_commit_job(JCB_t * pxJCB, int iJobPar, void * pxJobData,
                        StackType_t   * pThreadStack, int iStackSize,
                        JobHandlerEntry * pJobHandler, uint32_t xJobPriority,
                        JCB_ActOption_t autoAct)
{
  JCB_t * theJCB ;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

  // critical section enter   {{{
// Note:: It is may not necessary to put entire function within CRITICAL Section
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

  if(pxJCB == NULL)
  {
    theJCB = xpickFreeListJCB();
  }
  else
  {
    theJCB = pxJCB;
  }

  if(theJCB != NULL)
  {	// theJCB has to be valid before do anything

    theJCB->iJobPar = iJobPar ;
    theJCB->pJobData = pxJobData;
    theJCB->pThreadStack = pThreadStack;
    theJCB->stackSize = iStackSize;
    theJCB->threadEntryFunc = pJobHandler;
    theJCB->uxPriority = xJobPriority;
    theJCB->stateOfJcb = JCB_STATE_READY;
    theJCB->actOption = autoAct;
    theJCB->pxBindingObj =NULL;
    //if(pConditionToRun == NULL)
    {
#if     0
      if(deferTicks)
      {
        addSysDeferListJCB(theJCB, deferTicks);
      }
      else
#endif
      {
        addReadyListJCB(theJCB);
        // check to find if ReqSchedulerService needed
        rtos_kernel_scheduler_check();
      }
    }
    //else
    {
      // add the Job to the Condition List
    }
  }

  // critical section exit    }}}
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);

  return theJCB;
}
/// }}} rtos_commit_simplejob }}}


/// {{{  rtos_commit_deferjob    {{{
#include    "rtos_softtimer.h"
#include    "defer_jcb.h"

/// to commit a deferJob, it use sysSoftTimer to manage the defering action
//      and release the sysSoftTimer automaticlly when JCB moved to readyJCBList.
///  in order to set/config more precisely the SoftTimer, it need to go
///  through SoftTimer API
JCB_t * rtos_commit_deferjob(JCB_t * pxJCB, int iJobPar,
                             void * pxJobData,
                             StackType_t    * pThreadStack,
                             int iStackSize,
                             JobHandlerEntry    * pJobHandler,
                             uint32_t xJobPriority,
                             JCB_ActOption_t autoAct,
                             TickType_t  deferTicks)
{
    JCB_t * theJCB  = NULL;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    //  {{{ critical section enter  {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);


    if(deferTicks)
    {
        SoftTimer_t * theSoftTimer = xAcquireFreeSoftTimer();

        if(theSoftTimer != NULL)
        {
            theJCB = pxJCB;
            if(theJCB == NULL)
            {
                theJCB = xpickFreeListJCB();
            }

            if(theJCB != NULL)
            {
                theSoftTimer->actOption = AutoFree;
                theSoftTimer->pActionObj = (ListXItem_t *)theJCB;
                theSoftTimer->tmState = Stop_State;
                theSoftTimer->xTimerPeriodInTicks = deferTicks;

                theJCB->iJobPar = iJobPar ;
                theJCB->pJobData = pxJobData;
//                theJCB->pThread = NULL;
                theJCB->pThreadStack = pThreadStack;
                theJCB->stackSize = iStackSize;
                theJCB->threadEntryFunc = pJobHandler;
                theJCB->uxPriority = xJobPriority;
                theJCB->stateOfJcb = JCB_WAITINGSFTTIMER;
                theJCB->actOption = autoAct;
                theJCB->pxBindingObj =NULL;
                if (theSoftTimer != addToSoftTimerWaitingList(theSoftTimer ))
                {   // something wrong here.
                    while(1);
                }
            }
            else
            {   // failed to get JCB, free up SoftTimer and return
                xReleaseSoftTimer(theSoftTimer);
            }
        }
    }
    else
    {
        theJCB = rtos_commit_job(pxJCB, iJobPar, pxJobData, pThreadStack, iStackSize,
                                            pJobHandler, xJobPriority, autoAct);
    }

    // }}}  critical section exit    }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);

    { // sanity check

      if(origCriticalLevel != arch4rtos_iGetSysCriticalLevel())
      {
        do
        {
          arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
        } // while(0); //
        while(1); // to debug
        // while(origCriticalLevel != arch4rtos_iGetSysCriticalLevel())
      }
    }

    return theJCB;
}

/// }}}  rtos_commit_deferjob    }}}

/// {{{  rtos_commit_mutexjob    {{{
JCB_t * rtos_commit_mutexjob(JCB_t * pxJCB, int iJobPar, void * pxJobData,
                        StackType_t * pThreadStack, int iStackSize,
                        JobHandlerEntry * pJobHandler, uint32_t xJobPriority,
                        JCB_ActOption_t autoAct, Mutex_t * pMutex4Job)
{
    JCB_t * theJCB  = NULL;

    if(pMutex4Job != NULL)
    {
        Mutex_t *   theMutex = pMutex4Job;
        SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

        if(pxJCB == NULL)
        {
            theJCB = xpickFreeListJCB();
        }
        else
        {
            theJCB = pxJCB;
        }

        if(theJCB != NULL)
        {   // theJCB has to be valid before do anything

            theJCB->iJobPar = iJobPar ;
            theJCB->pJobData = pxJobData;
    //      theJCB->pThread = NULL;
            theJCB->pThreadStack = pThreadStack;
            theJCB->stackSize = iStackSize;
            theJCB->threadEntryFunc = pJobHandler;
            theJCB->uxPriority = xJobPriority;
            //theJCB->delayActive = deferTicks;
            theJCB->stateOfJcb = JCB_WAITINGMUTEX;

            theJCB->actOption = autoAct;
            theJCB->pxBindingObj = theMutex;
        }

        //  {{{ critical section enter  {{{
        arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
        if(theMutex->owner_thread == NULL)
        {   // theMutex has no Thread owner,
            // then deal with JobOwnder or WaitingList
            if(theMutex->owner_job == NULL)
            {
                theMutex->owner_job = theJCB;
                // add the JCB to ReadyList
                addReadyListJCB(theJCB);
            }
            else    // if(NULL != theMutex->owner_job)
            {
                if(theMutex->owner_job->uxPriority > theJCB->uxPriority)
                {   // move the original JobOwner to WaitingList
                    // and make the new owner_job
                    JCB_t   *  pxOrigOwnerJob = theMutex->owner_job;

                    pickReadyListJCB(pxOrigOwnerJob);
                    addJCBToMutexWaitingList(theMutex, pxOrigOwnerJob);
                    theMutex->owner_job = theJCB;
                    // add the JCB to ReadyList
                    addReadyListJCB(theJCB);
                }
                else
                {   // current JobOwner has same or higher priority
                    addJCBToMutexWaitingList(theMutex, theJCB);
                }
            }
        }
        else
        {   //  theMutex owned by a Thread, put the theJCB into waitingJobList
            addJCBToMutexWaitingList(theMutex, theJCB);
        }

        // at the end to check if need to arch4rtosReqSchedulerService(void);
        if((theMutex->owner_job != NULL) &&
                (theMutex->owner_job->uxPriority < pxCurrentTCB->uxPriority))
        {   // for the commit_conditionjob, it only need to check the owner_job
            // uxPriority
            arch4rtosReqSchedulerService();
        }
        // }}}  critical section exit    }}}
        arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    }
    else
    {
        theJCB = rtos_commit_job(pxJCB, iJobPar, pxJobData, pThreadStack, iStackSize,
                pJobHandler, xJobPriority, autoAct);
    }
    return theJCB;
}
/// }}}  rtos_commit_mutexjob    }}}

/// {{{ rtos_commit_semajob    {{{
JCB_t * rtos_commit_semjob(JCB_t * pxJCB, int iJobPar, void * pxJobData,
                        StackType_t * pThreadStack, int iStackSize,
                        JobHandlerEntry * pJobHandler, uint32_t xJobPriority,
                        JCB_ActOption_t autoAct, Sem_t *  pSem4Job)
{
    JCB_t * theJCB  = NULL;

    if(pSem4Job != NULL)
    {
        Sem_t *   theSem = pSem4Job;
        SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

        if(pxJCB == NULL)
        {
            theJCB = xpickFreeListJCB();
        }
        else
        {
            theJCB = pxJCB;
        }

        if(theJCB != NULL)
        {   // theJCB has to be valid before do anything

            theJCB->iJobPar = iJobPar ;
            theJCB->pJobData = pxJobData;
            theJCB->pThreadStack = pThreadStack;
            theJCB->stackSize = iStackSize;
            theJCB->threadEntryFunc = pJobHandler;
            theJCB->uxPriority = xJobPriority;
            theJCB->stateOfJcb = JCB_WAITINGSEMAG;

            theJCB->actOption = autoAct;
            theJCB->pxBindingObj = theSem;
        }

        //  {{{ critical section enter  {{{
        arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
        if(theSem->tokens) // != 0
        {

            theSem->tokens --;
            theJCB->pxBindingObj = NULL;
            // add the JCB to ReadyList
            addReadyListJCB(theJCB);
        }
        else    // if(theSem->tokens == 0)
        { // add JCB to SemWaitingList
            addJCBToSemWaitingList(theSem, theJCB);
        }

        {   // for the commit_conditionjob, it only need to check the
            // owner_job uxPriority
            arch4rtosReqSchedulerService();
        }
        // }}}  critical section exit    }}}
        arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    }
    else
    {
        theJCB = rtos_commit_job(pxJCB, iJobPar, pxJobData, pThreadStack,
                iStackSize, pJobHandler, xJobPriority, autoAct);
    }

    return theJCB;
}

/// }}}  rtos_commit_semjob    }}}

