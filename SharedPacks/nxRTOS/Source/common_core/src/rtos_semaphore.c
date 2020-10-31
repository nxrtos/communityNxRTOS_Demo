/* rtos_emaphore.c
 *
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

#include    "rtos_semaphore.h"
#include    "rtos_defer_tcb.h"
#include    "arch4rtos_criticallevel.h"
#include    "rtos_jcb.h"
#include    "list_tcb.h"
#include    "list_jcb.h"
#include    "nxRTOSConfig.h"


/// {{{ freeSoftTimerList   {{{
#if     (RTOS_SYS_SEMAPHORE_NUM)
static    Sem_t   resSysSemaphore[RTOS_SYS_SEMAPHORE_NUM];
#endif

/// }}} freeSoftTimerList   }}}
int     xFreeSemaNum = 0;


Sem_t     * pFreeSysSemaList = NULL;

static  int    addTCBToSemWaitingList(Sem_t    * theSem, R2BTCB_t * theTCB);

int     initSysSemFreeList()
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

     // critical section enter
     arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

#if     (RTOS_SYS_SEMAPHORE_NUM)
     resSysSemaphore[0].pXItem = NULL;
     for(int i = RTOS_SYS_SEMAPHORE_NUM -1; i > 0; i--)
     {
         resSysSemaphore[i].pXItem = (ListXItem_t *)&resSysSemaphore[i-1];
     }
     pFreeSysSemaList = &resSysSemaphore[RTOS_SYS_SEMAPHORE_NUM -1];
#endif

    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return retV;
}

Sem_t     * xSemNew(uint32_t max_count,  uint32_t initial_count)
{
    Sem_t     * theSem = NULL;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

     // critical section enter
     arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
     theSem = pFreeSysSemaList;
     if(NULL != theSem)
     {
         pFreeSysSemaList = (Sem_t *)theSem->pXItem;
         theSem->pXItem = NULL;
         theSem->pxWaitingJobList = NULL;
         theSem->max_tokens = max_count;
         theSem->tokens = initial_count;
     }
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return  theSem;
}

int xSemDelete(Sem_t * theSem)
{
    int     retV = 0;

    if(theSem != NULL)
    {
        SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

         // critical section enter
         arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
         theSem->pXItem = (ListXItem_t *)pFreeSysSemaList;
         pFreeSysSemaList = theSem;
         // critical section exit
         arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    }
    return retV;
}


int xSemRelease( Sem_t * theSem)
{
    int     retV = 0;

    if(theSem != NULL)
    {
        SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

         // critical section enter
         arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
         theSem->tokens += 1;
         if(theSem->tokens > theSem->max_tokens)
         {
             theSem->tokens = theSem->max_tokens;
         }

         // walk through WaitingList  if tokens > 0
         while (theSem->tokens > 0)
         {
             if((theSem->pxWaitingJobList == NULL) && (theSem->pXItem == NULL))
             {  // both pxWaitingJobList and pXItem == NULL, there are no waiter
                 break;
             }
             else
             if((theSem->pxWaitingJobList == NULL) && (theSem->pXItem != NULL))
             {
             R2BTCB_t   *   theTCB;
             theTCB = (R2BTCB_t   *)theSem->pXItem;
             theSem->tokens -= 1;
             // special case of
             // pickTCBFromSemWaitingList(Sem_t * theSem, R2BTCB_t * theTCB)
             // do short cut for performance optimize, as theTCB is the theSem->pXItem
             theSem->pXItem = (ListXItem_t *)((R2BTCB_t *)theSem->pXItem)->xEventListItem.next;

             theTCB->xEventListItem.prev = NULL;
             theTCB->xEventListItem.next = NULL;
#if     1
             //  if support timered acquire
             if((theTCB->wFlags & Wait4Ticks)  != 0)
             {  // the TCB is in TimeredWatingList
                 // pick it out
                 pickFromDeferTCBWaitingList(theTCB);
                 // clear the flag of Wait4Ticks to indicate the Thread _resume
                 // from a waiting Object  acquired, rather than timeout
                 theTCB->wFlags ^= Wait4Ticks;
             }
#endif
             addTCBToRun2BlckTCBList((BaseTCB_t *)theTCB);
             if((pxCurrentTCB != NULL) &&
                     (theTCB->baseTCB.uxPriority < pxCurrentTCB->uxPriority))
             {
                 arch4rtosReqSchedulerService();
             }
             }
             else
             if((theSem->pxWaitingJobList != NULL) && (theSem->pXItem == NULL))
             {  // only a JobWaiter
                 // so  theJCB win one count of  Sema
                 JCB_t          * theJCB = (JCB_t *)theSem->pxWaitingJobList;
                 theSem->tokens -= 1;
                 // special case of
                 // pickJCBFromSemWaitingList(Sem_t * theSem, JCB_t * theTCB)
                 // do short cut for performance optimize, as theJCB is the theSem->
                 theSem->pxWaitingJobList = (XCB_Base_t *)theJCB->xJcbListItem.next;
                 theJCB->xJcbListItem.next = NULL;
                 theJCB->xJcbListItem.prev = NULL;
                 // clear pxWaitingObj before put into ReadyListJCB
                 // to avoid treat pxWaitingObj as Mutex later on createTCB
                 theJCB->pxBindingObj = NULL;
                 addReadyListJCB(theJCB);
                 if((pxCurrentTCB != NULL) &&
                         (theJCB->uxPriority < pxCurrentTCB->uxPriority))
                 {
                     arch4rtosReqSchedulerService();
                 }
             }
             else
             {  // both JobWaiter and ThreadWaiter
                 R2BTCB_t * theTCB = (R2BTCB_t *)theSem->pXItem;
                 JCB_t          * theJCB = (JCB_t *)theSem->pxWaitingJobList;
                 if(theJCB->uxPriority < theTCB->baseTCB.uxPriority)
                 {  //  theJCB win one count of  Sema
                     theSem->tokens -= 1;
                     // special case of
                     // pickJCBFromSemWaitingList(Sem_t * theSem, JCB_t * theTCB)
                     // do short cut for performance optimize, as theJCB is the theSem->
                     theSem->pxWaitingJobList = (XCB_Base_t *)theJCB->xJcbListItem.next;
                     theJCB->xJcbListItem.next = NULL;
                     theJCB->xJcbListItem.prev = NULL;
                     // clear pxWaitingObj before put into ReadyListJCB
                     // to avoid treat pxWaitingObj as Mutex later on createTCB
                     theJCB->pxBindingObj = NULL;
                     addReadyListJCB(theJCB);
                     if((pxCurrentTCB != NULL) &&
                             (theJCB->uxPriority < pxCurrentTCB->uxPriority))
                     {
                         arch4rtosReqSchedulerService();
                     }
                 }
                 else
                 {  //  theTCB win one count of  Sema
                     theSem->tokens -= 1;
                     // special case of
                     // pickTCBFromSemWaitingList(Sem_t * theSem, R2BTCB_t * theTCB)
                     // do short cut for performance optimize, as theTCB is the theSem->pXItem
                     theSem->pXItem = (ListXItem_t *)((R2BTCB_t *)theSem->pXItem)->xEventListItem.next;

                     theTCB->xEventListItem.prev = NULL;
                     theTCB->xEventListItem.next = NULL;
        #if     1
                     //  if support timered acquire
                     if((theTCB->wFlags & Wait4Ticks)  != 0)
                     {  // the TCB is in TimeredWatingList
                         // pick it out
                         pickFromDeferTCBWaitingList(theTCB);
                         // clear the flag of Wait4Ticks to indicate the Thread _resume
                         // from a waiting Object  acquired, rather than timeout
                         theTCB->wFlags ^= Wait4Ticks;
                     }
        #endif
                     addTCBToRun2BlckTCBList((BaseTCB_t *)theTCB);
                     if((pxCurrentTCB != NULL) &&
                             (theTCB->baseTCB.uxPriority < pxCurrentTCB->uxPriority))
                     {
                         arch4rtosReqSchedulerService();
                     }
                 }
             }

         }
         // critical section exit
         arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    }
    return retV;
}

int xSemGetCount(Sem_t * theSem)
{
    int retV = 0;
    if(theSem != NULL)
    {
        retV = theSem->tokens;
    }
    return  retV;
}

Sem_t * pxSemAcquire(Sem_t * acqSem, TickType_t timeout)
{
    Sem_t * theSem = acqSem;

    if(theSem != NULL)
    {
        R2BTCB_t   *   theTCB;

        SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
        // critical section enter
        arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
        theTCB = (R2BTCB_t   *)pxCurrentTCB;

        if(acqSem->tokens)
        {
            acqSem->tokens -= 1;
        }
        else  if(timeout == 0)
        { // for timeout == 0 , it immediately  return NULL
            theSem = NULL;
        }
        else  //    timeout != 0
        {   // add theTCB into waitingList
            if(theTCB != NULL)
            {
                pickTCBFromRun2BlckTCBList((BaseTCB_t *)theTCB);
                theTCB->pxWaitingObj = acqSem;
                addTCBToSemWaitingList(acqSem, theTCB);
                if(timeout == WAIT_FOREVER)
                {
                    theTCB->wFlags = Wait4Sem;
                    theTCB->baseTCB.xStateListItem.next = NULL;
                    theTCB->baseTCB.xStateListItem.prev = NULL;
                }
                else
                {   // add theTCB to deferTCB as well
                    theTCB->xResumeOnTicks = timeout; // != 0
                    theTCB->wFlags = Wait4Ticks  | Wait4Sem;
                    addToDeferTCBWaitingList(theTCB);
                }
                // pxCurrentTCB went to waiting list, need to
                arch4rtosReqSchedulerService();
          }
          else
          {
              while(1);
          }
        }
        // critical section exit
        arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
        // after this point it means the Thread resumed if it has been
        //  in one or more waitingList
        if(theSem != NULL)
        {   // check the resume reason
            if(theTCB->wFlags & Wait4Ticks)
            {   // the flag of Wait4Ticks is set so it is from timeout
                theSem = NULL;  // return NULL to indicate timeout
                theTCB->wFlags ^= Wait4Ticks;
            }
        }
    }
    return theSem;
}

int    pickTCBFromSemWaitingList(Sem_t * theSem, R2BTCB_t * theTCB)
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(theSem != NULL)
    {
        if(theSem->pXItem == (ListXItem_t *)theTCB)
        {
            theSem->pXItem = theTCB->xEventListItem.next;
        }
        else
        {
            R2BTCB_t * refTCB = (R2BTCB_t *)theTCB->xEventListItem.prev;
            if(refTCB != NULL)
            {
                refTCB->xEventListItem.next = theTCB->xEventListItem.next;
            }
            refTCB = (R2BTCB_t *)theTCB->xEventListItem.next;
            if(refTCB != NULL)
            {
                refTCB->xEventListItem.prev = theTCB->xEventListItem.prev;
            }
        }
    }
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return retV;
}

static  int    addTCBToSemWaitingList(Sem_t * theSem, R2BTCB_t * theTCB)
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(theSem != NULL)
    {
        R2BTCB_t * refTCB;
        refTCB = (R2BTCB_t *)theSem->pXItem;

        if(refTCB == NULL)
        {
            theSem->pXItem = (ListXItem_t *)theTCB;
            theTCB->xEventListItem.next = NULL;
            theTCB->xEventListItem.prev = NULL;
        }
        else
        {
            do
            {
                if(refTCB->xEventListItem.next == NULL)
                {   // the refTCB is last one
                    break;
                }
                else if(refTCB->baseTCB.uxPriority <= theTCB->baseTCB.uxPriority)
                {   // move refTCB to next
                    refTCB = (R2BTCB_t *)refTCB->xEventListItem.next;
                }
            } while(1);

            if(refTCB->baseTCB.uxPriority <= theTCB->baseTCB.uxPriority)
            {   // add theTCB after refTCB
                theTCB->xEventListItem.prev = (ListXItem_t *)refTCB;
                theTCB->xEventListItem.next = refTCB->xEventListItem.next;
                refTCB->xEventListItem.next = (ListXItem_t *)theTCB;
            }
            else
            {// add theTCB before refTCB
                theTCB->xEventListItem.prev = refTCB->xEventListItem.prev;
                theTCB->xEventListItem.next = (ListXItem_t *)refTCB;
                refTCB->xEventListItem.prev = (ListXItem_t *)theTCB;
            }
        }

    }
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return retV;
}

// this most likely not needed, as pickJCB from SemWaitingList always from
// Head now.  If in the future to support cancelJobInSemaWaiting, then it need to
// pick JCB from anywhere in middle of SemWaitingList.
int    pickJCBFromSemWaitingList(Sem_t * theSem, JCB_t * theJCB)
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(theSem != NULL)
    {
        if(theSem->pxWaitingJobList == (XCB_Base_t *)theJCB)
        {
            theSem->pxWaitingJobList = (XCB_Base_t *)theJCB->xJcbListItem.next;
        }
        else
        {
            JCB_t * refJCB = (JCB_t *)theJCB->xJcbListItem.prev;
            if(refJCB != NULL)
            {
                refJCB->xJcbListItem.next = theJCB->xJcbListItem.next;
            }
            refJCB = (JCB_t *)theJCB->xJcbListItem.next;
            if(refJCB != NULL)
            {
                refJCB->xJcbListItem.prev = theJCB->xJcbListItem.prev;
            }
        }
    }
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return retV;
}

int    addJCBToSemWaitingList(Sem_t * theSem, JCB_t * theJCB)
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(theSem != NULL)
    {
        JCB_t * refJCB;
        refJCB = (JCB_t *)theSem->pxWaitingJobList;

        if(refJCB == NULL)
        {
            theSem->pxWaitingJobList = (XCB_Base_t *)theJCB;
            theJCB->xJcbListItem.next = NULL;
            theJCB->xJcbListItem.prev = NULL;
        }
        else
        {
            do
            {
                if(refJCB->xJcbListItem.next == NULL)
                {   // the refJCB is last one
                    break;
                }
                else if(refJCB->uxPriority <= theJCB->uxPriority)
                {   // move refTCB to next
                    refJCB = (JCB_t *)refJCB->xJcbListItem.next;
                }
            } while(1);

            if(refJCB->uxPriority <= theJCB->uxPriority)
            {   // add theJCB after refJCB
                theJCB->xJcbListItem.prev = (JCB_t *)refJCB;
                theJCB->xJcbListItem.next = refJCB->xJcbListItem.next;
                refJCB->xJcbListItem.next = (JCB_t *)theJCB;
            }
            else
            {// add theJCB before refJCB
                theJCB->xJcbListItem.prev = refJCB->xJcbListItem.prev;
                theJCB->xJcbListItem.next = (JCB_t *)refJCB;
                refJCB->xJcbListItem.prev = (JCB_t *)theJCB;
            }
        }
    }
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return retV;
}
