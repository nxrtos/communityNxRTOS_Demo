/* arch4rtos_criticallevel.h
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019 . or its affiliates.  All Rights Reserved.
 *
 *
 * 1 tab == 2 spaces!
 */

#ifndef ARCH4RTOS_CRITICAL_LEVEL_H
#define ARCH4RTOS_CRITICAL_LEVEL_H

#include  "arch_basetype.h"
#include  "CMSIS_Device.h"
#include  "arch4rtos_softbasepri.h"

#define     HIGHEST_SYSCRITICALLEVEL    (0)
#define     LOWEST_ISRCRITICALLEVEL     ((0x01 << __NVIC_PRIO_BITS) -1)
#define     LOWEST_SYSCRITICALLEVEL     (0x01 << __NVIC_PRIO_BITS)
#define     THREAD_SYSCRITICALLEVEL     LOWEST_SYSCRITICALLEVEL

#ifdef __cplusplus
extern "C" {
#endif

///{{{  SysCriticalLevel_API	{{{
extern SysCriticalLevel_t arch4rtos_iGetSysCriticalLevel(void);
extern SysCriticalLevel_t arch4rtos_iSetSysCriticalLevel(SysCriticalLevel_t newLevel);

extern SysCriticalLevel_t arch4rtos_iRaiseSysCriticalLevel(SysCriticalLevel_t newLevel);
extern SysCriticalLevel_t arch4rtos_iDropSysCriticalLevel(SysCriticalLevel_t newLevel);
///}}}	SysCriticalLevel_API	}}}

#ifdef __cplusplus
}
#endif

#endif /* ARCH4RTOS_CRITICAL_LEVEL_H */

