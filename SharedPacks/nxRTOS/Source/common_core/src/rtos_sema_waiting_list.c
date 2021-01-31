/**
  ******************************************************************************
  * @file           : rtos_sema_waiting_list.c
  * @brief          :
  * ****************************************************************************
  * @attention
  *                 nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */

/*-----------------------------------------------------------------------------
 * Semaphore control block
 *---------------------------------------------------------------------------*/
#include  "rtos_sema_waiting_list.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_jcb.h"
#include  "rtos_jcb_ready_list.h"
/**
  * @brief  Init Semaphore Waiting List within Semaphore Control Block
  *         This function is invoked when a new Semaphore creates.
  * @param  theSem, pointer to the Semaphore Control Block
  * @retval TBD. error code
  */
int initSemWaitList(Sema_t * theSem)
{
  int retV = 0;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // critical section enter
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  if(theSem != NULL)
  {
    theSem->pxWaitingBaseTCBList = NULL;
  }
  // critical section exit
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  return retV;
}

/**
  * @brief  Insert a JCB or TCB into Semaphore Waiting List.
  *         This function is invoked when a new Semaphore creates.
  * @param  theSem, pointer to the Semaphore Control Block, to which need to
  *         insert the JCB or TCB into Waiting List.
  *         theBaseTCB, pointer to the JCB or TCB which going to join into
  *         Waiting List of Semaphore.
  * @retval TBD. error code
  */
Sema_t * pxInsertToSemWaitList(Sema_t * theSem, BaseTCB_t * theBaseTCB)
{
  if((theSem != NULL) && (theBaseTCB != NULL))
  {
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
    // critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    // place other sanity check here if wanted, make sure theBaseTCB is
    // within pxWaitingBaseTCBList

    if(theSem->pxWaitingBaseTCBList == NULL)
    {
      theSem->pxWaitingBaseTCBList = theBaseTCB;
      theBaseTCB->xBTcbListItem.next = NULL;
      theBaseTCB->xBTcbListItem.prev = NULL;
    }
    else
    { // universal method to insert into List against priority order
      pxInsertToTCBList((BaseTCB_t * *)&(theSem->pxWaitingBaseTCBList), theBaseTCB);
    }
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    // critical section exit    }}}
  }
  return theSem;
}

Sema_t * pxRemoveFromSemWaitList(Sema_t * theSem, BaseTCB_t * theBaseTCB)
{
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // critical section enter   {{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

  if((theSem != NULL) && (theBaseTCB != NULL))
  { // place other sanity check here if wanted, make sure theBaseTCB is
    // within pxWaitingBaseTCBList
    pxRemoveFromTCBList((BaseTCB_t * *)&(theSem->pxWaitingBaseTCBList),
                                                                   theBaseTCB);
  }
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // critical section exit    }}}
  return theSem;
}

BaseTCB_t * pickHeadSemWaitList(Sema_t * theSem)
{
  BaseTCB_t * theBaseTCB = NULL;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();
  // critical section enter   {{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

  if(theSem != NULL)
  {
    theBaseTCB = theSem->pxWaitingBaseTCBList;
    if(theBaseTCB != NULL)
    {
      pxRemoveFromTCBList((BaseTCB_t * *)&(theSem->pxWaitingBaseTCBList),
                                                                   theBaseTCB);
    }
  }
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // critical section exit    }}}
  return theBaseTCB;
}










//// {{{  pending for removing {{{
#if   0
// this most likely not needed, as pickJCB from SemWaitingList always from
// Head now.  If in the future to support cancelJobInSemaWaiting, then it need to
// pick JCB from anywhere in middle of SemWaitingList.
int    pickJCBFromSemWaitingList(Sema_t * theSem, JCB_t * theJCB)
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(theSem != NULL)
    {
        if(theSem->pxWaitingBaseTCBList == (XCB_Base_t *)theJCB)
        {
            theSem->pxWaitingBaseTCBList = (XCB_Base_t *)theJCB->xJcbListItem.next;
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


int    addJCBToSemWaitingList(Sema_t * theSem, JCB_t * theJCB)
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(theSem != NULL)
    {
        JCB_t * refJCB;
        refJCB = (JCB_t *)theSem->pxWaitingBaseTCBList;

        if(refJCB == NULL)
        {
            theSem->pxWaitingBaseTCBList = (XCB_Base_t *)theJCB;
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


int    addTCBToSemWaitingList(Sema_t * theSem, R2BTCB_t * theTCB)
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(theSem != NULL)
    {
        R2BTCB_t * refTCB;
        refTCB = (R2BTCB_t *)theSem->pNextSema;

        if(refTCB == NULL)
        {
            theSem->pNextSema = theTCB;
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

int    pickTCBFromSemWaitingList(Sema_t * theSem, R2BTCB_t * theTCB)
{
    int retV = 0;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(theSem != NULL)
    {
        if(theSem->pNextSema == theTCB)
        {
            theSem->pNextSema = theTCB->xEventListItem.next;
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
#endif
//// }}}  pending for removing }}}
