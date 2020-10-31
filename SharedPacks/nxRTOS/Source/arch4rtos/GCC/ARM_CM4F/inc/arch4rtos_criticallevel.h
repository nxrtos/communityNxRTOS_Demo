/* arch4rtos_criticallevel.h
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019 . or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 2 spaces!
 */


#ifndef  ARCH4RTOS_SYSCRITICAL_LEVEL_H
#define  ARCH4RTOS_SYSCRITICAL_LEVEL_H

#if  0
#include  "arch4rtos_criticallevel.txt.h"
#endif

#define  TEST_SysCriticalLevel        01

#include  "arch_basetype.h"
#include  "CMSIS_Device.h"


#define  HIGHEST_SYSCRITICALLEVEL       (0)
#define  LOWEST_SYSCRITICALLEVEL        (0x01 << __NVIC_PRIO_BITS)
#define  THREAD_SYSCRITICALLEVEL        LOWEST_SYSCRITICALLEVEL
#define  LOWEST_ISRCRITICALLEVEL        ((0x01 << __NVIC_PRIO_BITS) -1)

#ifdef __cplusplus
extern "C" {
#endif

///{{{	SysCriticalLevel
extern
SysCriticalLevel_t arch4rtos_iGetSysCriticalLevel(void);
extern
SysCriticalLevel_t arch4rtos_iSetSysCriticalLevel(SysCriticalLevel_t newLevel);
extern
SysCriticalLevel_t arch4rtos_iRaiseSysCriticalLevel(SysCriticalLevel_t newLevel);
extern
SysCriticalLevel_t arch4rtos_iDropSysCriticalLevel(SysCriticalLevel_t newLevel);
///}}}

#if     defined(TEST_SysCriticalLevel)
extern int xTestSysCriticalLevelFromThread(void);
extern int xTestSysCriticalLevelFromISR(void);
#else
#define   xTestSysCriticalLevelFromThread()     do{}while(0)
#define   xTestSysCriticalLevelFromISR()        do{}while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* ARCH4RTOS_CRITICAL_LEVEL_H */

