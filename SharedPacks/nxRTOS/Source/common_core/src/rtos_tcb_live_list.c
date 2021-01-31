/**
  ******************************************************************************
  * @file           : rtos_tcb_list.c
  *
  * 1 tab == 2 spaces!
  ******************************************************************************
  */

#include  "rtos_tcb_live_list.h"
#include  "rtos_tcb_base_list.h"

LiveTCB_t * pxCurrentLiveTCB = NULL;

/// {{{ run2TermListTCB {{{
static LiveTCB_t * pxRun2TermTCBList_Head = NULL;

/// }}} run2TermListTCB }}}


/// {{{ run2BlckListTCB {{{
static LiveTCB_t * pxRun2BlckTCBList_Head = NULL;

#if (RTOS_TCB_DEBUG_TRACE)
static  int shortLive_TCB_num_trace = 0;
#endif

/// }}} run2BlckListTCB }}}


// in Run2XxxxTCBList, ->xTcbListItem.prev point to lower priority of TCB


LiveTCB_t * removeTCBFromRun2BlckTCBList(LiveTCB_t * theTCB)
#if  0
{
  if(theTCB != NULL)
  {
    // TODO, more check to make sure theTCB is in the Run2BlckTCBList
    LiveTCB_t * refTCB;

    refTCB = (LiveTCB_t *)theTCB->xTcbListItem.prev;
    if(refTCB != NULL)
    {
      refTCB->xTcbListItem.next = theTCB->xTcbListItem.next;
    }

    refTCB = (LiveTCB_t *)theTCB->xTcbListItem.next;
    if(refTCB != NULL)
    {
      refTCB->xTcbListItem.prev = theTCB->xTcbListItem.prev;
    }
    else
    { // this means theTCB is the HEAD
      if(theTCB != pxRun2BlckTCBList_Head)
      {
        while(1);
      }
      else
      { // update new pxRun2BlckTCBList_Head
        pxRun2BlckTCBList_Head = (LiveTCB_t *)theTCB->xTcbListItem.prev;
      }
    }
  }
  return theTCB;
}
#else
{ // use unified TCBList method
  LiveTCB_t * retTCB;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

  // {{{  critical section enter  {{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  #if (RTOS_TCB_DEBUG_TRACE)
    shortLive_TCB_num_trace--;
  #endif
  retTCB = (LiveTCB_t *)pxRemoveFromTCBList((BaseTCB_t * *)&pxRun2BlckTCBList_Head,
                                          (BaseTCB_t *)theTCB);
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // }}} critical section end }}}

  return retTCB;
}
#endif

LiveTCB_t * insertTCBToRun2BlckTCBList(LiveTCB_t * theTCB)
#if  0
{
  if(theTCB != NULL)
  {
    if(pxRun2BlckTCBList_Head == NULL)
    {
      pxRun2BlckTCBList_Head = theTCB;
      theTCB->xTcbListItem.next = NULL;
      theTCB->xTcbListItem.prev = NULL;
    }
    else
    {
      LiveTCB_t * refTCB;
      refTCB = pxRun2BlckTCBList_Head;
      do
      {
        if(refTCB->uxPriority > theTCB->uxPriority)
        { // add theTCB before refTCB
          theTCB->xTcbListItem.next = refTCB->xTcbListItem.next;
          theTCB->xTcbListItem.prev = refTCB;
          refTCB->xTcbListItem.next = theTCB;
          if(refTCB == pxRun2BlckTCBList_Head)
          { // new pxRun2BlckTCBList_Head
            pxRun2BlckTCBList_Head = theTCB;
          }
          if(theTCB->xTcbListItem.next != NULL)
          {
              refTCB = (LiveTCB_t *)theTCB->xTcbListItem.next;
              refTCB->xTcbListItem.prev = theTCB;
          }
          break;
        }
        else  // if(refTCB->uxPriority <= theTCB->uxPriority)
        { // move refTCB to .prev
          if(refTCB->xTcbListItem.prev != NULL)
          {
            refTCB = (LiveTCB_t *)refTCB->xTcbListItem.prev;
          }
          else
          { // add theTCB after refTCB aka the last in the RunXxxList
            theTCB->xTcbListItem.next = refTCB;
            theTCB->xTcbListItem.prev = refTCB->xTcbListItem.prev; // NULL
            refTCB->xTcbListItem.prev = theTCB;
            break;
          }
        }
      }while(1);
    }
  }
  return theTCB;
}
#else
{
  LiveTCB_t * retTCB;
  SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

  // {{{  critical section enter  {{{
  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
  #if (RTOS_TCB_DEBUG_TRACE)
    shortLive_TCB_num_trace++;
  #endif

  retTCB =  (LiveTCB_t *)pxInsertToTCBList((BaseTCB_t * *)&pxRun2BlckTCBList_Head,
                                      (BaseTCB_t *)theTCB);
  arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
  // }}} critical section end }}}
  return retTCB;
}
#endif

LiveTCB_t * pushRun2TermListTCB(LiveTCB_t * toList)
#if  0
{
  if(toList == pxRun2TermTCBList_Head)
  {
    return toList;
    while(1);
  }
  if(toList != NULL)
  {
    toList->xTcbListItem.prev = NULL;
    toList->xTcbListItem.next = pxRun2TermTCBList_Head;
    if(pxRun2TermTCBList_Head != NULL)
    { // for Run2TermTCBList it is not necessary but just for consistent
      //  to Run2TermTCBList  and debug
      pxRun2TermTCBList_Head->xTcbListItem.prev = toList;
    }
    pxRun2TermTCBList_Head = toList;
  }

  return toList;
}
#elif 0
{
  return  (LiveTCB_t *)pushToTCBList((BaseTCB_t * *)&pxRun2TermTCBList_Head,
                                      (BaseTCB_t *)toList);
}
#else
{
  return  (LiveTCB_t *)pxInsertToTCBList((BaseTCB_t * *)&pxRun2TermTCBList_Head,
                                      (BaseTCB_t *)toList);
}
#endif

LiveTCB_t * popRun2TermListTCB()
#if 01
{
  LiveTCB_t * theTCB = pxRun2TermTCBList_Head;
  if(theTCB != NULL)
  {
    pxRun2TermTCBList_Head = (LiveTCB_t *)theTCB->xTcbListItem.next;
    if(NULL != pxRun2TermTCBList_Head)
    {
      pxRun2TermTCBList_Head->xTcbListItem.prev =NULL;
    }
  }
  return theTCB;
}
#elif 0
{ // use unified TCBList method
  return (LiveTCB_t *)popFromTCBList((BaseTCB_t * *)&pxRun2TermTCBList_Head);
}
#else
{ // use unified TCBList method
  return (LiveTCB_t *)pxRemoveFromTCBList((BaseTCB_t * *)&pxRun2TermTCBList_Head,
                                          (BaseTCB_t *)pxRun2TermTCBList_Head);
}
#endif

LiveTCB_t * getCurrentRun2TermTCB()
{
    return  pxRun2TermTCBList_Head;
}

LiveTCB_t * getCurrentRun2BlkTCB()
{
    return  pxRun2BlckTCBList_Head;
}

LiveTCB_t * getCurrentTCB()
{
    return  pxCurrentLiveTCB;
}

LiveTCB_t * updateCurrentTCB()
{
  if(NULL == pxRun2TermTCBList_Head)
  {
    pxCurrentLiveTCB = pxRun2BlckTCBList_Head;
  }
  else if (NULL == pxRun2BlckTCBList_Head)
  {
    pxCurrentLiveTCB = pxRun2TermTCBList_Head;
  }
  else
  {
    if(pxRun2TermTCBList_Head->uxPriority < pxRun2BlckTCBList_Head->uxPriority)
    {
      pxCurrentLiveTCB = pxRun2TermTCBList_Head;
    }
    else if(pxRun2TermTCBList_Head->uxPriority > pxRun2BlckTCBList_Head->uxPriority)
    {
      pxCurrentLiveTCB = pxRun2BlckTCBList_Head;
    }
    else //
    if(pxCurrentLiveTCB != pxRun2BlckTCBList_Head)
    {
      pxCurrentLiveTCB = pxRun2TermTCBList_Head;
    }
  }
  return  pxCurrentLiveTCB;
}
