/**
  ******************************************************************************
  * @file           : rtos_tcb_live_list.h
  *
  ******************************************************************************
  *
  * nxRTOS Kernel V0.0.1
  * Copyright (C) 2019  nxrtos or its affiliates.  All Rights Reserved.
  *
  * 1 tab == 2 spaces!
 **/
#if 0
  #include  "rtos_tcb_list.txt.h"
#endif
/*-----------------------------------------------------------------------------
 * Implementation of functions for job control
 *---------------------------------------------------------------------------*/

#ifndef  RTOS_TCB_LIST_H    //  {{{
#define  RTOS_TCB_LIST_H    //  {{{

#include  "list.h"
#include  "rtos_tcb_live.h"

/// {{{ run2TermListTCB {{{
extern  LiveTCB_t * pushRun2TermListTCB(LiveTCB_t * toList);
extern  LiveTCB_t * popRun2TermListTCB(void);
extern  LiveTCB_t * getCurrentRun2TermTCB();
/// }}} run2TermListTCB }}}

LiveTCB_t * insertTCBToRun2BlckTCBList(LiveTCB_t * theTCB);
LiveTCB_t * removeTCBFromRun2BlckTCBList(LiveTCB_t * theTCB);
LiveTCB_t * getCurrentRun2BlkTCB();
LiveTCB_t * getCurrentTCB(void);
LiveTCB_t * updateCurrentTCB();

#endif  // }}}  RTOS_TCB_LIST_H }}}
