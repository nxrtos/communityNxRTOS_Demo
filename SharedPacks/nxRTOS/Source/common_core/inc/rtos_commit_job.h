/* rtos_commit_job.h
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
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
 * is inactive until the threads above it all went terminated.
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
 * A special Init_Thread is a type of run2TermThread, started by startRTOS()
 * through SysInitJob.  At the end of Init_Thread, it need to commit a
 * repeating Idle_Processig_Job, which is causing Idle_run2TermThread to
 * executed in exclusive lowest priority repeatedly when no other running
 * Thread or ready Job available.
 *
 *---------------------------------------------------------------------------*/

#ifndef	COMMIT_JOB_H
#define COMMIT_JOB_H

#if  0
#include  "rtos_commit_job.txt.h"
#endif

#include  "rtos_jcb.h"

/// {{{ rtos_commit_job {{{
extern
JCB_t * rtos_commit_job(JCB_t * pxJCB, int iJobPar, void * pxJobData,
                        StackType_t   * pThreadStack, int iStackSize,
                        JobHandlerEntry * pJobHandler, uint32_t xJobPriority,
                        JCB_ActOption_t autoAct);
extern
JCB_t * rtos_commit_deferjob(JCB_t * pxJCB, int iJobPar, void * pxJobData,
                        StackType_t * pThreadStack, int iStackSize,
                        JobHandlerEntry * pJobHandler, uint32_t xJobPriority,
                        JCB_ActOption_t autoAct, TickType_t  deferTicks);

#include  "rtos_mutex.h"
extern
JCB_t * rtos_commit_mutexjob(JCB_t * pxJCB, int iJobPar, void * pxJobData,
                        StackType_t * pThreadStack, int iStackSize,
                        JobHandlerEntry * pJobHandler, uint32_t xJobPriority,
                        JCB_ActOption_t autoAct, Mutex_t * pMutex4Job);
// for rtos_commit_semjob(...) see
#include  "rtos_semaphore.h"
/// }}} rtos_commit_job }}}

#endif
