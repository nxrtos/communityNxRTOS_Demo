/**
  ******************************************************************************
  * @file           : rtos_tcb_base_list.h
  * @brief          : define Thread Control Block Base type
  ******************************************************************************
  * @attention
  * nxRTOS Kernel V0.0.1
  * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
  *
  * 1 tab == 2 spaces!
  */

#ifndef  _RTOS_TCB_BASE_LIST_H    //  {{{
#define  _RTOS_TCB_BASE_LIST_H    //  {{{

#include  "list.h"
#include  "rtos_tcb_base.h"

// unified operation over TCBList
BaseTCB_t * pxInsertToTCBList(BaseTCB_t * * pTcbListHead, BaseTCB_t * pTheTCB);

BaseTCB_t * pxRemoveFromTCBList(BaseTCB_t * * pTcbListHead, BaseTCB_t * pTheTCB);

BaseTCB_t * pushToTCBList(BaseTCB_t * * pTcbListHead, BaseTCB_t * pTheTCB);
BaseTCB_t * popFromTCBList(BaseTCB_t * * pTcbListHead);


#endif  //  }}} _RTOS_TCB_BASE_LIST_H }}}
