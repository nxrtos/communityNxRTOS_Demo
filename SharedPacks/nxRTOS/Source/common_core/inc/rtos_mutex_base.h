/**
  ******************************************************************************
  * @file           : rtos_mutex_base.h
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
#ifndef __RTOS_MUTEX_BASE_H  //  {{{
#define __RTOS_MUTEX_BASE_H  //  {{{
#if  0
#include  "rtos_mutex_base.txt.h"
#endif
#include  "arch4rtos_basedefs.h"
#include  "rtos_xcb_base.h"
#include  "rtos_tcb_base.h"
#include  "rtos_tcb_live.h"
#include  "rtos_jcb.h"

#ifdef __cplusplus
extern "C" {
#endif
/// {{{ Mutex attributes {{{
#if  (SUPPORT_MUTEX_ATTRIBUTES)
typedef enum _mutex_attr_bit_pos
{
#if  (SUPPORT_MUTEX_ATTR_RECURSIVE)
  osMutexRecursive_Pos, // = 0,
#endif
#if  (SUPPORT_MUTEX_ATTR_PRIO_INHRT)
  osMutexPrioInherit_Pos, // = 1,
#endif
#if  (SUPPORT_MUTEX_ATTR_AUTORELS)
  osMutexRobust_Pos, // = 3,
#endif
  osMutexBorder_Pos   // the border position
} osMutexAttr_Pos;

typedef enum _mutex_attr_type
{
  osMutexAttrAllClear = 0,
  // {{{ all between only list partial to be used as element to OR {{{
#if  (SUPPORT_MUTEX_ATTR_RECURSIVE)
  osMutexRecursive = 0x1 << osMutexRecursive_Pos,
#endif
#if  (SUPPORT_MUTEX_ATTR_PRIO_INHRT)
  osMutexPrioInherit = 0x1 << osMutexPrioInherit_Pos,
#endif
#if  (SUPPORT_MUTEX_ATTR_AUTORELS)
  osMutexRobust = 0x1 << osMutexRobust_Pos,
#endif
  // }}} all between only list partial to be used as element to OR }}}
  osMutexAttrAllSet = (0x1 << osMutexBorder_Pos) -1
} osMutexAttr_t;
/*
  *   osMutexRecursive : a thread can consume the mutex multiple times without
  *                       locking itself.
  *                      in order to support this feature, need 'Lock counter',
  *                      a 'uint8_t lock;' in Mutex_t or osRtxMutex_t
  *   osMutexPrioInherit : priority of a waiting thread is raised (when lower)
  *                       to priority of mutex owner thread.
  *                      in order to support this feature, need 'Base Priority'
  *                      and 'Thread Priority' in osRtxThread_t or LiveThread_t.
  *                      The essential needs is that the capability to track all
  *                      potential source to promote the runtime ThreadPriority.
  *                      in nxRTOS the source could be 1.) own Mutexes for which
  *                      are requiring by other Threads with higher runtime
  *                      Priority. 2.) if it is Shortlive_Thread, the priority
  *                      of stackedUnder_Shortlive_Thread has been promoted
  *                      Priority and so propagated the promotion.
  *
  *   osMutexRobust : the mutex is automatically released when owner thread
  *                   is terminated.
*/
#endif
/// }}} Mutex attributes }}}

typedef struct  _mutex_struct
{
  Xcb_Base_Items; //  common part shared among many of Control Block
  BaseTCB_t * pxWaitingBaseTCBList; // point to the Mutex waitList,
                                //  contains mixed TCBs and JCBs
  LiveTCB_t * owner_thread;  ///< Owner Thread
#if  (SUPPORT_MUTEX_ATTRIBUTES)
  osMutexAttr_t mutex_attr;
#endif
#if  (SUPPORT_MUTEX_ATTR_RECURSIVE)
  uint8_t lock; // track recursive count, ref to osRtxMutex_t
#endif
} Mutex_t;

#ifdef __cplusplus
}
#endif

#endif	// }}}  __RTOS_MUTEX_BASE_H }}}
