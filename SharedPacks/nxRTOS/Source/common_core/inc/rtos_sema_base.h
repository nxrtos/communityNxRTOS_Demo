/**
  ******************************************************************************
  * @file           : rtos_sema_base.h
  * @brief          :
  * ****************************************************************************
  * @attention
  *                 nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */

/*-----------------------------------------------------------------------------
 * define Mutex control block, the base type
 *---------------------------------------------------------------------------*/
// {{{
#ifndef __RTOS_SEMAPHORE_BASE_H  //  {{{
#define __RTOS_SEMAPHORE_BASE_H  //  {{{
#if  0
#include  "rtos_sema_base.txt.h"
#endif
#include  "arch4rtos_basedefs.h"
#include  "rtos_xcb_base.h"
#include  "rtos_tcb_base.h"
#include  "rtos_tcb_live.h"
#include  "rtos_jcb.h"

#ifdef __cplusplus
extern "C" {
#endif
/// {{{ Semaphore attributes {{{
#if  (SUPPORT_SEMA_ATTRIBUTES)
typedef enum _sema_attr_bit_pos
{
  osSemaBorder_Pos   // the border position
} osSemaAttr_Pos;

typedef enum _sema_attr_type
{
  osSemaAttrAllClear = 0,
  // {{{ all between only list partial to be used as element to OR {{{
  // }}} all between only list partial to be used as element to OR }}}
  osSemaAttrAllSet = (0x1 << osSemaBorder_Pos) -1
} osSemaAttr_t;
#endif
/// }}} Semaphore attributes }}}

typedef struct  _semaphore_struct
{
  Xcb_Base_Items;  ///< Common part shared among many of Control Block
  uint16_t  tokens;  ///< Current number of tokens
  uint16_t  max_tokens;  ///< Maximum number of tokens
  BaseTCB_t * pxWaitingBaseTCBList; // point to the sema waitList,
                                //  contains mixed TCBs and JCBs
#if  (SUPPORT_SEMA_ATTRIBUTES)
  osSemaAttr_t sema_attr;
#endif
} Sema_t;
#ifdef __cplusplus
}
#endif
#endif	// }}}  __RTOS_SEMAPHORE_BASE_H }}}
