/* arch4rtos_testcriticallevel.h
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019 . or its affiliates.  All Rights Reserved.
 *
 *
 * 1 tab == 2 spaces!
 */

#ifndef ARCH4RTOS_TESTCRITICAL_LEVEL_H
#define ARCH4RTOS_TESTCRITICAL_LEVEL_H

#define   TEST_SysCriticalLevel             01

#include  "arch4rtos_criticallevel.h"
#include  "nxRTOSConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int xTestSysCriticalLevelFromThread(void);
extern int xTestSysCriticalLevelFromISR(void);

#ifdef __cplusplus
}
#endif

#endif /* ARCH4RTOS_TESTCRITICAL_LEVEL_H */

