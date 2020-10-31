/* rtos_mutex.c
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

#include    "rtos_mutex.h"
#include    "rtos_defer_tcb.h"
#include    "arch4rtos_criticallevel.h"
#include    "rtos_jcb.h"
#include    "list_jcb.h"
#include    "list_tcb.h"
#include    "nxRTOSConfig.h"


/// {{{ freeSoftTimerList   {{{
#if     (RTOS_SYS_MUTEX_NUM)
static    Mutex_t   resSysMutex[RTOS_SYS_MUTEX_NUM];
#endif

/// }}} freeSoftTimerList   }}}
int     xFreeMutexNum = 0;

Mutex_t     * pFreeSysMutexList = NULL;

static  int    addTCBToMutexWaitingList(Mutex_t    * theMutex, R2BTCB_t * theTCB);
static  int    addMutexToThreadHoldingList(Mutex_t * theMutex, R2BTCB_t * theTCB);
static  int    removeMutexFromThreadHoldingList(Mutex_t * theMutex, R2BTCB_t * theTCB);

int     initSysMutexFreeList()
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

     // critical section enter
     arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

#if     (RTOS_SYS_MUTEX_NUM)
     resSysMutex[0].pXItem = NULL;
     for(int i = RTOS_SYS_MUTEX_NUM -1; i > 0; i--)
     {
         resSysMutex[i].pXItem = (ListXItem_t *)&resSysMutex[i-1];
     }
     pFreeSysMutexList = &resSysMutex[RTOS_SYS_MUTEX_NUM -1];
#endif

    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return retV;
}

Mutex_t     * xMutexNew(Mutex_t * theMutex)
{
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

     // critical section enter
     arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

     if(theMutex == NULL)
     {
         theMutex = pFreeSysMutexList;
         if(theMutex != NULL)
         {
             pFreeSysMutexList = (Mutex_t *)theMutex->pXItem;
         }
     }

     if(NULL != theMutex)
     {
         theMutex->job_waiting_list = NULL;
         theMutex->owner_job = NULL;
         theMutex->owner_thread = NULL;
         theMutex->pXItem = NULL;
     }
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return  theMutex;
}

int xMutexDelete(Mutex_t * theMutex)
{
    int     retV = 0;

    if(theMutex != NULL)
    {
        SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

         // critical section enter
         arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
         theMutex->pXItem = (ListXItem_t *)pFreeSysMutexList;
         theMutex->owner_job = NULL;
         theMutex->owner_thread = NULL;
         theMutex->job_waiting_list = NULL;
         pFreeSysMutexList = theMutex;
         // critical section exit
         arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    }
    return retV;
}


int xMutexRelease(Mutex_t * theMutex)
{
    int     retV = 0;

    if(theMutex != NULL)
    {
        SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

         // critical section enter
         arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

         {
             // remove theMutex from pxOwnedObjList
             Mutex_t * refMutex = pxCurrentTCB->pxOwnedObjList;

             if(refMutex == theMutex)
             {  // the refMutex is the one to release, update pxOwnedObjList
                 pxCurrentTCB->pxOwnedObjList = refMutex->pxNextMutex;
             }
             else
             {
                 while(refMutex != NULL)
                 {
                     if(refMutex->pxNextMutex == theMutex)
                     {
                         refMutex->pxNextMutex = theMutex->pxNextMutex;
                         break;
                     }
                 }

             }
         }

         if(theMutex->owner_thread == (R2BTCB_t *)pxCurrentTCB)
         {
             theMutex->owner_thread = NULL;
             // now check   waitingList
             if(theMutex->job_waiting_list == NULL)
             {  // no job waiting , check ThreadWaiting only
                 R2BTCB_t * newOwnerThread = (R2BTCB_t * )theMutex->pXItem;

                 if(newOwnerThread != NULL)
                 {  // new owner_thread
                     pickTCBFromMutexWaitingList(theMutex, newOwnerThread);
                     if(newOwnerThread->wFlags & Wait4Ticks)
                     {  // if the newOwnerThread is waiting for Ticks as well,
                         // remove it from DeferTCBWaitingList
                         pickFromDeferTCBWaitingList(newOwnerThread);
                         // clear the Wait4Ticks in wFlags    by flip it.
                         newOwnerThread->wFlags ^= Wait4Ticks;
                     }
                     // put the Thread into
                     addTCBToRun2BlckTCBList((BaseTCB_t *) newOwnerThread);
                 }
                 theMutex->owner_thread = newOwnerThread;
             }
             else if(theMutex->pXItem == NULL)
             {  //  && (theMutex->job_waiting_list != NULL)
                 theMutex->owner_job = theMutex->job_waiting_list;
                 theMutex->job_waiting_list = (JCB_t *)theMutex->owner_job->
                                                             xJcbListItem.next;
                 // add the JCB to ReadyList
                 addReadyListJCB(theMutex->owner_job);
             }
             else // there are JCB and TCB wait for this Mutex. find high priority
             {    //if(theMutex->pXItem != NULL) && (theMutex->job_waiting_list != NULL)
                 if(theMutex->job_waiting_list->uxPriority < ((R2BTCB_t * )
                                         theMutex->pXItem)->baseTCB.uxPriority)
                 {  // job win the competing for Mutex ,
                     theMutex->owner_job = theMutex->job_waiting_list;
                     theMutex->job_waiting_list = (JCB_t *)theMutex->owner_job->
                                                                 xJcbListItem.next;
                     // add the JCB to ReadyList
                     addReadyListJCB(theMutex->owner_job);
                 }
                 else
                 {  // R2BTCB_thread win competing for Mutex ,
                     R2BTCB_t * newOwnerThread = (R2BTCB_t * )theMutex->pXItem;

                     if(newOwnerThread != NULL)
                     {  // new owner_thread
                         pickTCBFromMutexWaitingList(theMutex, newOwnerThread);
                         if(newOwnerThread->wFlags & Wait4Ticks)
                         {  // if the newOwnerThread is waiting for Ticks as well,
                             // remove it from DeferTCBWaitingList
                             pickFromDeferTCBWaitingList(newOwnerThread);
                             // clear the Wait4Ticks in wFlags    by flip it.
                             newOwnerThread->wFlags ^= Wait4Ticks;
                         }
                         // put the Thread into
                         addTCBToRun2BlckTCBList((BaseTCB_t *) newOwnerThread);
                     }
                     theMutex->owner_thread = newOwnerThread;
                 }
             }

             if((theMutex->owner_thread != NULL) &&
                     (theMutex->owner_thread->baseTCB.uxPriority < pxCurrentTCB->uxPriority))
             {
                 // pxCurrentTCB went to waiting list, need to
                 arch4rtosReqSchedulerService();
             }
             // else  // little optimization
             if((theMutex->owner_job != NULL) &&
                     (theMutex->owner_job->uxPriority < pxCurrentTCB->uxPriority))
             {
                 // pxCurrentTCB went to waiting list, need to
                 arch4rtosReqSchedulerService();
             }
         }
         // critical section exit
         arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    }
    return retV;
}

void *  xMutexGetOwner(Mutex_t * theMuex)
{
    void *  theOwner;

    theOwner = theMuex->owner_job;
    if(NULL ==theOwner)
    {
        theOwner = theMuex->owner_thread;
    }
    return theOwner;
}

// this mostly only tell if there is a waiter.
void *       xMutexGetWaiter(Mutex_t * theMuex)
{
    void *  theWaiter;

    theWaiter = theMuex->pXItem;
    if(NULL ==theWaiter)
    {
        theWaiter = theMuex->job_waiting_list;
    }
    return theWaiter;
}

void *       xMutexGetThreadWaiter(Mutex_t * theMuex)
{
    void *  theWaiter;
    theWaiter = theMuex->pXItem;
    return theWaiter;
}

void *       xMutexGetJobWaiter(Mutex_t * theMuex)
{
    void *  theWaiter;
    theWaiter = theMuex->job_waiting_list;
    return theWaiter;
}

Mutex_t *   pxThreadMutexAcquire(Mutex_t * acqMutex, TickType_t timeout)
{
    Mutex_t *   theMutex = acqMutex;
    R2BTCB_t *  theTCB = (R2BTCB_t   *)pxCurrentTCB;

    if(acqMutex != NULL)
    {
        SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

        // critical section enter
        arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
        if((acqMutex->owner_job == NULL) && (acqMutex->owner_thread == NULL))
        {
            addMutexToThreadHoldingList(acqMutex, theTCB);
        }
        else if(acqMutex->owner_thread == NULL)
        {   // if currentlly the owner is a JCB, aka  acqMutex->owner_job,
            // then grab it and send it back to waitingList
            JCB_t * theOrigOwnerJCB = acqMutex->owner_job;
            pickReadyListJCB(theOrigOwnerJCB);
            acqMutex->owner_job = NULL;
            theOrigOwnerJCB->xJcbListItem.next = NULL;
            theOrigOwnerJCB->xJcbListItem.prev = NULL;
            addJCBToMutexWaitingList(acqMutex, theOrigOwnerJCB);
            addMutexToThreadHoldingList(acqMutex, theTCB);
        }
        else // The Mutex currently is hold by other running TCB
        if(timeout == 0)
        {   // failed to acquire Mutex when no wait
            theMutex = NULL;
            theTCB->pxWaitingObj = NULL;
            theTCB->wFlags = 0;
            theTCB->xEventListItem.next = NULL;
            theTCB->xEventListItem.prev = NULL;
            theTCB->xResumeOnTicks = 0;
        }
        else // timeout != 0
        {
            R2BTCB_t   *   theTCB =  (R2BTCB_t   *)pxCurrentTCB;

            // add theTCB into waitingList
            if(theTCB != NULL)
            {
                pickTCBFromRun2BlckTCBList((BaseTCB_t *)theTCB);
                addTCBToMutexWaitingList(acqMutex, theTCB);
                theTCB->pxWaitingObj = acqMutex;
                if(timeout == WAIT_FOREVER)
                {
                    theTCB->wFlags = Wait4Mutex;
                    theTCB->baseTCB.xStateListItem.next = NULL;
                    theTCB->baseTCB.xStateListItem.prev = NULL;
                }
                else
                {   // add theTCB to deferTCB as well
                    theTCB->xResumeOnTicks = timeout; // != 0
                    theTCB->wFlags = Wait4Ticks  | Wait4Mutex;
                    addToDeferTCBWaitingList(theTCB);
                }

                {// promote acqMutex owner's priority
                    if(acqMutex->owner_thread != NULL)
                    {
                        if(acqMutex->owner_thread->baseTCB.uxPriority
                                            < theTCB->baseTCB.uxPriority)
                        {
                            acqMutex->owner_thread->baseTCB.uxPriority
                                            = theTCB->baseTCB.uxPriority;
                        }
                    }
                    else
                    {   // catch errror
                        while(1);
                    }
                }
                // pxCurrentTCB went to waiting list, need to
                arch4rtosReqSchedulerService();
            }
            else
            {   // error catch  pxCurrentTCB == NULL
                while(1);
            }
            // pxCurrentTCB went to waiting list, need to
            arch4rtosReqSchedulerService();
        }
          // critical section exit
          arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
          // after this point it means the Thread resumed if it has been
          //  in one or more waitingList
          if(theMutex != NULL)
          {   // check the resume reason
              if(theTCB->wFlags & Wait4Ticks)
              {   // the flag of Wait4Ticks is set so it is from timeout
                  theMutex = NULL;  // return NULL to indicate timeout
                  theTCB->wFlags ^= Wait4Ticks;
              }
          }
     }
     return theMutex;
}

Mutex_t *   pxJobMutexAcquire(Mutex_t * theMuex, JCB_t * theJCB)
{
    if(theMuex != NULL)
    {
        SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

         // critical section enter
         arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);





         // critical section exit
         arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    }
    return theMuex;
}

static  int    addTCBToMutexWaitingList(Mutex_t * theMutex, R2BTCB_t * theTCB)
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(theMutex != NULL)
    {
        // it more likely to add the currentTCB the theMutex WaitingList
        // and  currentTCB will be highest Priority than other TCB in WiatingList if there is any
        // any exception case ?
        theTCB->xEventListItem.prev = NULL;
        theTCB->xEventListItem.next = theMutex->pXItem;
        theMutex->pXItem = (ListXItem_t *)theTCB;
    }
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return retV;
}

// this very similar to pickTCBFromSemWaitingList, consider to optimize in the future
int    pickTCBFromMutexWaitingList(Mutex_t * theMutex, R2BTCB_t * theTCB)
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(theMutex != NULL)
    {
        if(theMutex->pXItem == (ListXItem_t *)theTCB)
        {
            theMutex->pXItem = theTCB->xEventListItem.next;
            theTCB->xEventListItem.next = NULL; // clear theTCB
            if(theTCB->xEventListItem.prev != NULL)
            {   // catch error
                while(1);
            }
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
            theTCB->xEventListItem.next = NULL; // clear theTCB
            theTCB->xEventListItem.prev = NULL;
        }
    }
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return retV;
}

// As JCB can't be in DederWaitingList and MutexWaitingList at the same time,
// the xTheListItem member  used for  JCB_MutexWaitingList
//static
int    addJCBToMutexWaitingList(Mutex_t * theMutex, JCB_t * theJCB)
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(theMutex != NULL)
    {
        JCB_t * refJCB;
        refJCB = theMutex->job_waiting_list;

        if(refJCB == NULL)
        {
            theMutex->job_waiting_list = theJCB;
            theJCB->xJcbListItem.next = NULL;
            theJCB->xJcbListItem.prev = NULL;
        }
        else
        {
            do
            {
                if(refJCB->xJcbListItem.next == NULL)
                {   // the refTCB is last one
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

//static
int    pickJCBFromMutexWaitingList(Mutex_t * theMutex, JCB_t * theJCB)
{
    int retV = 0;
     SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

     // critical section enter
     arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
     if(theMutex != NULL)
     {
         if(theMutex->job_waiting_list == theJCB)
         {
             theMutex->job_waiting_list = (JCB_t *)theJCB->xJcbListItem.next;
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

static  int    addMutexToThreadHoldingList(Mutex_t * theMutex, R2BTCB_t * theTCB)
{
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

    theMutex->owner_thread = theTCB;
    theMutex->pxNextMutex = theTCB->baseTCB.pxOwnedObjList;

    if(theTCB->baseTCB.pxOwnedObjList == NULL)
    {   // this is first Mutex acquired by Thread, so update
        theTCB->baseTCB.uxOrigPriority = theTCB->baseTCB.uxPriority;
    }
    theTCB->baseTCB.pxOwnedObjList = theMutex;
    theTCB->wFlags = 0;
    theTCB->xEventListItem.next = NULL;
    theTCB->xEventListItem.prev = NULL;
    theTCB->xResumeOnTicks = 0;

    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return 0;
}

static  int    removeMutexFromThreadHoldingList(Mutex_t * theMutex, R2BTCB_t * theTCB)
{
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

    if(theTCB->baseTCB.pxOwnedObjList == theMutex)
    {
        theTCB->baseTCB.pxOwnedObjList = theMutex->pxNextMutex;
    }
    else
    {
        Mutex_t * refMutex = theTCB->baseTCB.pxOwnedObjList;
        do
        {
            if(refMutex->pxNextMutex == theMutex)
            {
                refMutex->pxNextMutex = theMutex->pxNextMutex;
                break;
            }
            else
            {
                refMutex = refMutex->pxNextMutex;
                if(refMutex == NULL)
                {   // catch error
                    while(1);
                }
            }
        }while(1);
    }
    // clear the owner in theMutex
    if(theMutex->owner_thread == theTCB)
    {
        theMutex->owner_thread = NULL;
        theMutex->pxNextMutex = NULL;
    }
    else
    {   // catch error
        while(1);
    }

    {   // update     theTCB->baseTCB.uxPriority =
        Mutex_t * holdMutex = theTCB->baseTCB.pxOwnedObjList;
        ThreadPriorityType_t newPriority = theTCB->baseTCB.uxOrigPriority;
        do
        {   // walk through
            if(holdMutex != NULL)
            {
                if(holdMutex->pXItem != NULL)
                {
                    if(newPriority > ((R2BTCB_t *)holdMutex->pXItem)->baseTCB.uxPriority)
                    {
                        newPriority = ((R2BTCB_t *)holdMutex->pXItem)->baseTCB.uxPriority;
                    }
                }
                else if(holdMutex->owner_job != NULL)
                {
                    if(newPriority > holdMutex->owner_job->uxPriority)
                    {
                        newPriority = holdMutex->owner_job->uxPriority;
                    }
                }
                holdMutex = holdMutex->pxNextMutex;
            }
        }while(holdMutex != NULL);
        theTCB->baseTCB.uxPriority = newPriority;
    }
    // critical section exit
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return 0;
}

//  this usually used for a Thread going to termination and so need to release
//  all resource(Mutes for now) .
int    cleanAllMutexFromHoldingList(R2BTCB_t * theTCB)
{
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{  critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

    do
    {
        if(theTCB->baseTCB.pxOwnedObjList == NULL)
        {
            break;
        }
        else
        {
            //removeMutexFromThreadHoldingList((Mutex_t *)theTCB->baseTCB.pxOwnedObjList, theTCB);
            // TODO may optimize later
            Mutex_t * releasingMutex = theTCB->baseTCB.pxOwnedObjList;
            xMutexRelease(releasingMutex);
        }
    }while(theTCB->baseTCB.pxOwnedObjList != NULL);

    // }}}  critical section exit   }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return 0;
}
