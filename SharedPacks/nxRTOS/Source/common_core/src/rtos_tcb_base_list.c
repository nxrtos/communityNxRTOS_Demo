/**
  ******************************************************************************
  * @file           : rtos_tcb_base_list.c
  * @brief          : define Thread Control Block Base type
  ******************************************************************************
  * @attention
  * nxRTOS Kernel V0.0.1
  * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
  *
  * 1 tab == 2 spaces!
  */

#include  "list.h"
#include  "rtos_tcb_base.h"
#include  "rtos_tcb_base_list.h"

// unified operation over TCBList
BaseTCB_t * pxInsertToTCBList(BaseTCB_t * * pTcbListHead, BaseTCB_t * pTheTCB)
{
  if((pTheTCB != NULL) && (pTcbListHead != NULL))
  {
    if(*pTcbListHead == NULL)
    {
      (*pTcbListHead) = pTheTCB;
      pTheTCB->xTcbListItem.next = NULL;
      pTheTCB->xTcbListItem.prev = NULL;
    }
    else  //  if(pTcbListHead != NULL)
    {
      BaseTCB_t * pRefTCB = *pTcbListHead;
      do
      {
        if(pTheTCB->uxPriority < pRefTCB->uxPriority)
        { // add TheTCB before RefTCB
          pTheTCB->xBTcbListItem.next = pRefTCB;
          pTheTCB->xBTcbListItem.prev = pRefTCB->xBTcbListItem.prev;
          pRefTCB->xBTcbListItem.prev = pTheTCB;
          if(pRefTCB == *pTcbListHead)
          { // new TcbListHead
            *pTcbListHead = pTheTCB;
          }
          break;
        }
        else  // if(pTheTCB->uxPriority >= pRefTCB->uxPriority)
        { // move refTCB to .next
          if(pRefTCB->xTcbListItem.next != NULL)
          {
            pRefTCB = pRefTCB->xBTcbListItem.next;
          }
          else  // RefTCB is the tail
          { // add TheTCB after RefTCB aka the last in the in TcbList
            pTheTCB->xBTcbListItem.prev = pRefTCB;
            pTheTCB->xBTcbListItem.next = pRefTCB->xBTcbListItem.next; //NULL
            pRefTCB->xBTcbListItem.next = pTheTCB;
            break;
          }
        }
      }while(1);
      // by now TheTCB inserted into TcbList
    }
  }
  else
  {
    // invalid par, catch error?
    while(1);
  }
  return  pTheTCB;
}

BaseTCB_t * pxRemoveFromTCBList(BaseTCB_t * * pTcbListHead, BaseTCB_t * pTheTCB)
{
  if(pTheTCB != NULL)
  {
    if(pTheTCB->xTcbListItem.prev != NULL)
    {
      BaseTCB_t * pRefTCB = pTheTCB->xBTcbListItem.prev;
      pRefTCB->xTcbListItem.next = pTheTCB->xTcbListItem.next;
    }
    if(pTheTCB->xTcbListItem.next != NULL)
    {
      BaseTCB_t * pRefTCB = pTheTCB->xBTcbListItem.next;
      pRefTCB->xTcbListItem.prev = pTheTCB->xTcbListItem.prev;
    }
    if((pTcbListHead != NULL) && (* pTcbListHead == pTheTCB))
    {
      * pTcbListHead = pTheTCB->xBTcbListItem.next;
    }
  }

  if(pTcbListHead == NULL)
    return  NULL;
  else
    return  (* pTcbListHead);
}

BaseTCB_t * pushToTCBList(BaseTCB_t * * pTcbListHead, BaseTCB_t * pTheTCB)
{
  if((pTheTCB != NULL) && (pTcbListHead != NULL))
  {
    if(pTheTCB == (* pTcbListHead))
    {
      return pTheTCB;
      while(1);
    }
    if(pTheTCB != NULL)
    {
      pTheTCB->xBTcbListItem.prev = NULL;
      pTheTCB->xBTcbListItem.next = (* pTcbListHead);
      if((* pTcbListHead) != NULL)
      { // for Run2TermTCBList it is not necessary but just for consistent
        //  to Run2TermTCBList  and debug
        (* pTcbListHead)->xBTcbListItem.prev = (BaseTCB_t *)pTheTCB;
      }
      (* pTcbListHead) = pTheTCB;
    }
  }
  return pTheTCB;
}

BaseTCB_t * popFromTCBList(BaseTCB_t * * pTcbListHead)
{
  BaseTCB_t * theTCB = (* pTcbListHead);
  if(theTCB != NULL)
  {
    (* pTcbListHead) = (BaseTCB_t *)theTCB->xBTcbListItem.next;
    if(NULL != (* pTcbListHead))
    {
      (* pTcbListHead)->xBTcbListItem.prev =NULL;
    }
  }
  return theTCB;
}
