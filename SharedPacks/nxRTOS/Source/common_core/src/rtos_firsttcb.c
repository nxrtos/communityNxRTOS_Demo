/**
  ******************************************************************************
  * @file           : arch4rtos_firsttcb.c
  *
  * 1 tab == 2 spaces!
  ******************************************************************************
  */

#include  "arch4rtos.h"
#include  "arch4rtos_firstjob.h"
#include  "rtos_start_kernel.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_commit_job.h"
#include  "rtos_create_tcb.h"
#include  "rtos_tcb_live_list.h"

// refer to TCB_t * pickTCB(TCB_t * currentTCB )
LiveTCB_t *	firstTCB(JCB_t * theJCB)
{
  LiveTCB_t * theTCB;
  SysCriticalLevel_t  origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

  // {{{ raise Sys_Critical_Level  to designed Kernel_Critical_Level {{{
  // arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  arch4rtos_iRaiseSysCriticalLevel(HIGHEST_SYSCRITICALLEVEL);

  { // sanity check may put here for debugging.
    if(getCurrentTCB() != NULL)
      while(1);
    if(getCurrentRun2TermTCB() != NULL)
      while(1);
    if(getCurrentRun2BlkTCB() != NULL)
      while(1);
    // more check added after
  }

  // load_job_to_thread(getReadyJCB());
  theTCB = rtos_create_tcb(theJCB);
  if(theTCB == NULL)
  {
    while(1);
  }
  // update getCurrentRun2BlkTCB() or getCurrentRun2TermTCB()
  if(theJCB->pThreadStack ==ThreadStackTempStacking)
  { // getCurrentRun2TermTCB() = theTCB;
    pushRun2TermListTCB(theTCB);
  }
  else // if(theJCB->pThreadStack == StackKept)
  { // getCurrentRun2BlkTCB() = theTCB;
    insertTCBToRun2BlckTCBList(theTCB);
  }

  // update getCurrentTCB() here.
  //getCurrentTCB() = theTCB;
  updateCurrentTCB();
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // }}}restore Sys_Critical_Level to original }}}
  return theTCB;
}
