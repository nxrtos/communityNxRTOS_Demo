/* nxRTOSConfig.h
 * Kernel Config for nxRTOS V0.0.1
 * Copyright (C) 2019 , Inc. or its affiliates.  All Rights Reserved.
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
 * *
 * 1 tab == 4 spaces!
 */
#ifndef NXRTOS_CONFIG_H
#define NXRTOS_CONFIG_H

#include "preRTOSConfigDefs.h"


#define configSYSTICK_RATE_HZ               (1000)

#define configSOFTBASEPRI_Enable            0

// by default Enable_SHORTLIVETHREAD_Preemption is 1
#define configEnable_Preemption_SHORTLIVETHREAD  0

/// {{{     sysCriticalLevel    {{{
// follow Cortex-M scheme highest sysCriticalLevel is 0
//                    and lowest  sysCriticalLevel is (0x01 << __NVIC_PRIO_BITS)
// when configSOFTBASEPRI_Enable is 0,
// will configRTOSCriticalLevel must be HIGHEST_SYSCRITICALLEVEL
#define configRTOSCriticalLevel             HIGHEST_SYSCRITICALLEVEL
// 0
//((0x01 << __NVIC_PRIO_BITS) /2)
#define configMemCriticalLevel              configRTOSCriticalLevel
/// }}}     sysCriticalLevel    }}}

/// {{{     ThreadPriority      {{{
// highest Thread Priority is same as lowest  sysCriticalLevel,
// aka. (0x01 << __NVIC_PRIO_BITS) . Small number presents higher Priority
// configIdlePriority defines lowest Thread Priority in the system and only
// Idle Thread can assign this ThreadPriority
#define configIdlePriority                  ((0x01 << __NVIC_PRIO_BITS)  + 16)

/// }}}     ThreadPriority      }}}


#define configPREALLOCJCBS                          (32)
#define configRTOS_PREALLOC_SOFTTIMER_NUM           16

/// {{{ Stack Options   {{{
#define configISRStackSize                          0x200
#define configInitThreadStackSize                   0x400
#define configRTOS_IDLE_THREAD_STACKSIZE            0x400
/// Minimal StackSize for ShortLiveThread
#define configMinimalThreadStackSize                0x180
/// define LongLiveThreadStackPoolSize 0 to chose not support LongLiveThread
#define configLongLiveThreadStackPoolSize           0
/// Dynamic LongLiveThreadStackPool
//  #define configDynamicLongLiveThreadStackPool
#define configMinimalTaskStackSize                  0x180

/// {{{ THREAD_STACK_OVERFLOW_TRACE  {{{
//  default enable, set 0 to disable
#define configTHREAD_STACK_OVERFLOW_TRACE			1
/// }}} THREAD_STACK_OVERFLOW_TRACE  }}}

/// }}} Stack Options   }}}


#define configSYS_SEMAPHORE_NUM                     4


// SUPPORT_MUTEX by default, set 0 to disable
// #define configSUPPORT_MUTEX                         1

#define configSYS_MUTEX_NUM                         4

// by default support RTOS_Delay() in task, set 0 to disable
//#define configSUPPORT_DELAY_IN_THREAD               0

#include  "postRTOSConfigFixup.h"
#endif	// NXRTOS_CONFIG_H
