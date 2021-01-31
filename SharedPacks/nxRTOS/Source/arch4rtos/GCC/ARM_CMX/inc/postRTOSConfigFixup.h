/*
 * nxRTOS Kernel V0.0.1		postRTOSConfigFixup.h
 * Copyright (C) 2019 nxrtos or its affiliates.  All Rights Reserved.
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


#ifndef  _DEFUALT_RTOSCONFIG_FIXUP_H
#define  _DEFUALT_RTOSCONFIG_FIXUP_H

// help eclipse index
#ifndef   NXRTOS_CONFIG_H
#include  "nxRTOSConfig.h"
#endif

#include  "arch4rtos_criticallevel.h"
#include  "postRTOSConfigArchFixup.h"

/// {{{   THREAD_PRIORITY   {{{
#if (configIdlePriority < (0x01 << __NVIC_PRIO_BITS))
#error  "configIdlePriority can't be less than (0x01 << __NVIC_PRIO_BITS)"
#else
		// minimal valid RTOS_THREAD_PRIORITY_NUM is 2, as App_Priority must higher than configIdlePriority
#define  RTOS_HIGHEST_THREAD_PRIORITY       (0x01 << __NVIC_PRIO_BITS)
#define  RTOS_LOWEST_THREAD_PRIORITY        configIdlePriority
#define  RTOS_TERMINATION_THREAD_PRIORITY   (configIdlePriority +1)
#define  RTOS_NORMAPP_THREAD_PRIORITY       ((RTOS_HIGHEST_THREAD_PRIORITY + RTOS_LOWEST_THREAD_PRIORITY) /2)
#define  RTOS_THREAD_PRIORITY_NUM           (RTOS_LOWEST_THREAD_PRIORITY -RTOS_LOWEST_THREAD_PRIORITY +1)
#endif
/// }}}   THREAD_PRIORITY   }}}

/// {{{   STACK_Defs   {{{

#ifdef  configInitThreadStackSize
  #define  RTOS_INIT_THREAD_STACKSIZE       configInitThreadStackSize
#else
  #error   "Need to define configInitThreadStackSize in nxRTOSConfig.h"
#endif

#ifdef  configISRStackSize
  #define  RTOS_ISR_Stack_SIZE              configISRStackSize
#else
  #define  RTOS_ISR_Stack_SIZE                   0x100 /* in bytes */
#endif

/// by default ShortLiveThreadStackPool start from reset SP,
#define RTOS_INITIAL_RUN2TERM_ThreadSP      (*((StackType_t *)(0)))

/// and ShortLiveThreadStackPool may set as dynamic border

/// {{{ MINIMAL_THREAD_STACKSIZE is for ShortLiveThread  {{{
#ifdef   configMinimalThreadStackSize
  #define  RTOS_MINIMAL_THREAD_STACKSIZE    configMinimalThreadStackSize
#else
  #define  RTOS_MINIMAL_THREAD_STACKSIZE         0x100
#endif
/// }}} MINIMAL_THREAD_STACKSIZE is for ShortLiveThread  }}}

#ifdef	configLongLiveThreadStackPoolSize
  #define RTOS_RUN2BLCK_STACKPOOLSIZE       configLongLiveThreadStackPoolSize	/* in bytes */
  /// define to 0 means does not support LongLiveThread
#else
  #error  "need to define configLongLiveThreadStackPoolSize in nxRTOSConfig.h"
#endif

/// MinimalTaskStackSize is for LongLiveThread
#ifdef  configMinimalTaskStackSize
  #define RTOS_MINIMAL_TASK_STACKSIZE       configMinimalTaskStackSize
#endif
/// }}}   STACK_Defs   }}}

///  {{{  RTOS_SYS_JCB_NUM   {{{
#ifdef      configPREALLOCJCBS
#define     RTOS_SYS_JCB_NUM            configPREALLOCJCBS
#else
#define     RTOS_SYS_JCB_NUM            256
#endif
///  }}}  RTOS_SYS_JCB_NUM   }}}

///  {{{  RTOS_DEFER_JCB_NUM   {{{
#if     defined(configMaxDeferJobSupported)

#define configMaxDeferJobSupported
#define RTOS_DEFER_JCB_NUM          configMaxDeferJobSupported
#else
#define RTOS_DEFER_JCB_NUM          (RTOS_SYS_JCB_NUM/2)
#endif
///  }}}  RTOS_DEFER_JCB_NUM   }}}

///  {{{ SYS_SOFTTIMER_NUM   {{{
#if     defined(configRTOS_PREALLOC_SOFTTIMER_NUM)
#define SYS_SOFTTIMER_NUM           configRTOS_PREALLOC_SOFTTIMER_NUM
#else
#define SYS_SOFTTIMER_NUM           0
#endif
///  }}}  SYS_SOFTTIMER_NUM   }}}

#if  defined(configTHREAD_STACK_OVERFLOW_TRACE)
  #define  RTOS_THREAD_STACK_OVERFLOW_TRACE    configTHREAD_STACK_OVERFLOW_TRACE
#else
  #define RTOS_THREAD_STACK_OVERFLOW_TRACE     1
#endif


#if  defined(configSUPPORT_MUTEX)
  #define RTOS_SUPPORT_MUTEX                     configSUPPORT_MUTEX
#else
  #define RTOS_SUPPORT_MUTEX                     1
#endif

#if  defined(configSYS_SEMAPHORE_NUM)
  #define  RTOS_SYS_SEMAPHORE_NUM    configSYS_SEMAPHORE_NUM
#else
  #define  RTOS_SYS_SEMAPHORE_NUM    0
#endif

#if  defined(configSYS_MUTEX_NUM)
  #define  RTOS_SYS_MUTEX_NUM        configSYS_MUTEX_NUM
#else
  #define  RTOS_SYS_MUTEX_NUM        0
#endif

/// support RTOS_Delay for LongLive_Thread.  TBD
#if  defined(configSUPPORT_DELAY_IN_THREAD)
  #define  RTOS_SUPPORT_DELAY_IN_THREAD           configSUPPORT_DELAY_IN_THREAD
#else
  #define  RTOS_SUPPORT_DELAY_IN_THREAD           1
#endif

#ifdef  configEnable_Preemption_SHORTLIVETHREAD
  #undef  Enable_SHORTLIVETHREAD_Preemption
  #if  (configEnable_Preemption_SHORTLIVETHREAD != 0)
    #warning "community version does not support SHORTLIVETHREAD_Preemption"
  #endif
#else  // if no configEnable_Preemption_SHORTLIVETHREAD, undef it
  #undef Enable_SHORTLIVETHREAD_Preemption
#endif

#ifdef  configRTOS_IDLE_THREAD_STACKSIZE
#define RTOS_IDLE_THREAD_STACKSIZE      configRTOS_IDLE_THREAD_STACKSIZE
#else
#define RTOS_IDLE_THREAD_STACKSIZE      0
#endif

// RTOS_SVC_ENTRY_PRIORITY must to be between RTOS_SYSCRITICALLEVEL and
//  LOWEST_ISRCRITICALLEVEL inclusive.
#if  defined(configSVC_ENTRY_PRIORITY)
#if (configSVC_ENTRY_PRIORITY < RTOS_SYSCRITICALLEVEL)
#warning  "configSVC_ENTRY_PRIORITY exceed RTOS_SYSCRITICALLEVEL"
#define RTOS_SVC_ENTRY_PRIORITY         RTOS_SYSCRITICALLEVEL
#elif (configSVC_ENTRY_PRIORITY > LOWEST_ISRCRITICALLEVEL)
#warning  "configSVC_ENTRY_PRIORITY exceed LOWEST_ISRCRITICALLEVEL"
#define RTOS_SVC_ENTRY_PRIORITY         LOWEST_ISRCRITICALLEVEL
#else
#define RTOS_SVC_ENTRY_PRIORITY         configSVC_ENTRY_PRIORITY
#endif
#else
// set default RTOS_SVC_ENTRY_PRIORITY to RTOS_SYSCRITICALLEVEL for safe.
#define RTOS_SVC_ENTRY_PRIORITY         RTOS_SYSCRITICALLEVEL
//#define RTOS_SVC_ENTRY_PRIORITY         LOWEST_ISRCRITICALLEVEL
#endif

// ENABLE_SUPPORT_SEMA
#if   defined(configSupportSemaphoreConditionJob)
  #if  (configSupportSemaphoreConditionJob)
    #error	"community version does NOT support SEMA_CONDITION_JOB"
  #endif
  #define SUPPORT_SEMA_CONDITION_JOB        configSupportSemaphoreConditionJob
#else
// community version does NOT support SEMA_CONDITION_JOB
#define SUPPORT_SEMA_CONDITION_JOB        0
#endif

#if   SUPPORT_SEMA_CONDITION_JOB
#define SUPPORT_SEMA_ATTR_JOB_REASSIGN    01

//#define SUPPORT_SEMA_ATTR_JOB_REASSIGN 01
#if   (SUPPORT_SEMA_ATTR_JOB_REASSIGN)   || (0)
  #define SUPPORT_SEMA_ATTRIBUTES         01
#else
  #define SUPPORT_SEMA_ATTRIBUTES         0
#endif
#endif

// ENABLE_SUPPORT_MUTEX
#if   defined(configSupportMutexConditionJob)
  #if  (configSupportMutexConditionJob)
    #error	"community version does NOT support MUTEX_CONDITION_JOB"
  #endif
  #define SUPPORT_MUTEX_CONDITION_JOB       configSupportMutexConditionJob
#else
  // community version does NOT support MUTEX_CONDITION_JOB
  #define SUPPORT_MUTEX_CONDITION_JOB       0
#endif

#if !(SUPPORT_MUTEX_CONDITION_JOB)
#define SUPPORT_MUTEX_ATTR_JOB_REASSIGN   0
#else
#define SUPPORT_MUTEX_ATTR_JOB_REASSIGN   01
#endif

#define SUPPORT_MUTEX_ATTR_RECURSIVE    01
#define SUPPORT_MUTEX_ATTR_PRIO_INHRT   01
#define SUPPORT_MUTEX_ATTR_AUTORELS     01

#if   (SUPPORT_MUTEX_ATTR_JOB_REASSIGN)   \
      || (SUPPORT_MUTEX_ATTR_RECURSIVE)   \
      || (SUPPORT_MUTEX_ATTR_PRIO_INHRT)  \
      || (SUPPORT_MUTEX_ATTR_AUTORELS)
  #define SUPPORT_MUTEX_ATTRIBUTES        01
#else
  #define SUPPORT_MUTEX_ATTRIBUTES        0
#endif

#define RTOS_JCB_DEBUG_TRACE            1

#define RTOS_TCB_DEBUG_TRACE            1
#endif /* _DEFUALT_RTOSCONFIG_FIXUP_H */
