/**
 ******************************************************************************
 * @file      rtos_picktcb_postterm.c
 * @author
 * @brief     check tcb and the contains in stack
 *
 ******************************************************************************
 * @attention
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 2 spaces!
 */

#include  "rtos_picktcb_postterm.h"

/**
  * @brief pick a new TCB after terminate current Thread
  * @param theTermTCB, the TCB going to be terminated
  * @retval the live TCB to be current Thread
  *
  * Note:: invoke from SVC_FF_handler()
  */

LiveTCB_t *	pickTCB_PostTerm(LiveTCB_t * theTermTCB)
{
    LiveTCB_t * pxSelectToRunTCB;
    LiveTCB_t * theTCB;
    SysCriticalLevel_t  origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{ raise Sys_Critical_Level  to designed Kernel_Critical_Level {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    //arch4rtos_iRaiseSysCriticalLevel(0);
    theTCB = getCurrentTCB();

    while(theTermTCB != theTCB)
      theTermTCB = getCurrentTCB(); // sanity check

#ifdef  RTOS_THREAD_STACK_OVERFLOW_TRACE
    if(theTermTCB != NULL)  //  check stack overflow before Thread termination
    {
      if(*(theTermTCB->pxStackMarkPosition) != RTOS_STACK_OVERFLOW_TRACE_DEFAULTMARK)
      {   // failed on Mark check, stack has overflow
        while(1);
      }
    }
#endif
    // remove it from runningList
    if(theTermTCB == getCurrentRun2TermTCB())
    {   // if getCurrentTCB() is a Run2TermTCB, it must be the Head
        popRun2TermListTCB();
        //getCurrentRun2TermTCB() = (TCB_t *) getCurrentRun2TermTCB()->xTcbListItem.prev;
    }
    else if(theTermTCB == getCurrentRun2BlkTCB())
    {  // otherwise getCurrentTCB() must be Run2BlckTCB
        removeTCBFromRun2BlckTCBList(theTermTCB);
        //getCurrentRun2BlkTCB() = (TCB_t *) getCurrentRun2BlkTCB()->xTcbListItem.prev;
    }
    else
    { // catch error. can theTermTCB to be NULL?
      while(1);
    }

    // now look through getCurrentRun2TermTCB() , getCurrentRun2BlkTCB() and readyJCB
    // below will be similar to Scheduler_ pickTCB.
    // find the highest priority in readyJob and runningThread.
    // the highest priority of runningThread can be find from pxSelectToRunTCB.
    // it can could be NULL if getCurrentTCB() is going to terminating itself and
    // it is the highest priority of runningThread.
    // otherwise, it should be either same as getCurrentRun2TermTCB() or
    //  getCurrentRun2BlkTCB().

    // so first re-elect pxSelectToRunTCB among getCurrentRun2TermTCB()
    // getCurrentRun2BlkTCB()
    // calculate pxSelectToRunTCB
    {
      pxSelectToRunTCB = getCurrentRun2TermTCB();
      if(pxSelectToRunTCB ==  NULL)
      {
        pxSelectToRunTCB = getCurrentRun2BlkTCB();
      }
      else if(getCurrentRun2BlkTCB() != NULL)
      {   // neither getCurrentRun2BlkTCB() or getCurrentRun2TermTCB() be NULL
        if(getCurrentRun2TermTCB()->uxPriority > getCurrentRun2BlkTCB()->uxPriority)
        {
          pxSelectToRunTCB = getCurrentRun2BlkTCB();
        }
      }
    }
    // at this point pxSelectToRunTCB has elected among getCurrentRun2TermTCB()
    // getCurrentRun2BlkTCB().  Now need to check with getReadyJCB()
    if(getReadyJCB() == NULL)
    {   // pxSelectToRunTCB has not to be NULL, or SysIdleJob take over.
        if(pxSelectToRunTCB == NULL)
        {
            while(1); // replace by load SysIdleJob
            //pxSelectToRunTCB = getCurrentTCB();
        }
    }
    else if((pxSelectToRunTCB == NULL) || (getReadyJCB()->uxPriority < pxSelectToRunTCB->uxPriority))
    {   // readyJCB has higher priority, so load the Job and replace pxSelectToRunTCB
        JCB_t * theJCB;

        //theJCB = pxRemoveFromReadyJCBList(getReadyJCB()); // the pick will update getReadyJCB()
        theJCB = getReadyJCB();
        // load_job_to_thread(getReadyJCB());
        pxSelectToRunTCB = rtos_create_tcb(theJCB);

        if(pxSelectToRunTCB == NULL)
        {   // failed to rtos_create_tcb, more likely because of no enough space
            // for the StackSpace , put back the theJCB to ReadyList
            //pxInsertToReadyJCBList(theJCB);
            if(theJCB->pThreadStack == ThreadStackTempStacking)
            {   // the readyJCB is StackShared, promote
                if( getCurrentRun2TermTCB() == NULL)
                {   // theJCB has require stack_space too big to satisfy
                    while(1);
                }
                else
                {
                    if(getCurrentRun2TermTCB()->uxPriority > theJCB->uxPriority)
                    {   // promote the
                        getCurrentRun2TermTCB()->uxPriority = theJCB->uxPriority;
                        //getCurrentRun2TermTCB()->uxRtosPromotePriority = theJCB->uxPriority;
                    }
                    else
                    {   // seems something wrong
                        while(1);
                    }
                }
            }
        }
        else
        { // success to put theJCB into execution
          pxRemoveFromReadyJCBList(theJCB);
          // update getCurrentRun2BlkTCB() or getCurrentRun2TermTCB()
          //  and pxSelectToRunTCB
          if(theJCB->pThreadStack ==ThreadStackTempStacking)
          {
            pushRun2TermListTCB(pxSelectToRunTCB);
          }
          else // if(theJCB->pThreadStack == StackKept)
          {
            insertTCBToRun2BlckTCBList(pxSelectToRunTCB);
          }
        }
    }

    {   // in general at this moment should see theTCB == getCurrentTCB()
        //  and going to update getCurrentTCB() to new value. But the assumption
        // may change in the future if needed.
        UBaseType_t  xReadyJobPriority = RTOS_TERMINATION_THREAD_PRIORITY;
        UBaseType_t  xToRunThreadPriority = RTOS_TERMINATION_THREAD_PRIORITY;

        if (pxSelectToRunTCB != NULL)
        {
            xToRunThreadPriority = pxSelectToRunTCB->uxPriority;
        }

        if (getReadyJCB() != NULL)
        {
            xReadyJobPriority = getReadyJCB()->uxPriority;
        }

        if(xReadyJobPriority < xToRunThreadPriority)
        {
            // pick the JCB and prepare TCB and Thread_stack for it

        }
        else
        {
            if(xToRunThreadPriority != RTOS_TERMINATION_THREAD_PRIORITY)
            {   // pick theTCB  from RunningThread
                theTCB = pxSelectToRunTCB;
            }
            else
            {  // in real case can't be here, as it means both readyJobList and
               // toRunTCB are empty. seems system going shutdown.
               // or define a SysIdle_Job to handle this situation.
              while(1);
            }
        }

    }

    // update getCurrentTCB() here.
    //getCurrentTCB() = theTCB;
    updateCurrentTCB();

#ifdef  RTOS_THREAD_STACK_OVERFLOW_TRACE
    if(NULL != getCurrentRun2TermTCB())
    {  // to resumed Shotlive_Thtread
    	if(getCurrentRun2TermTCB()->pxStackMarkPosition)
    	{  // restore the StackBorderMark
    		*(getCurrentRun2TermTCB()->pxStackMarkPosition) =
                            RTOS_STACK_OVERFLOW_TRACE_DEFAULTMARK;
    	}

    }
#endif

    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // }}}restore Sys_Critical_Level to original }}}

    /// this will be a arch specific function to sanity check theTCB and
    //  contains in stack
    arch4rtos_check_TCB(theTCB);
    return theTCB;
}
