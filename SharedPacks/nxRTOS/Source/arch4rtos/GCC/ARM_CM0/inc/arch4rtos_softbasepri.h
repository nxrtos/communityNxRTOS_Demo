/* arch4rtos_softbasepri.h
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019 . or its affiliates.  All Rights Reserved.

 * 1 tab == 2 spaces!
 */


#ifndef ARCH4RTOS_SOFTBASEPRI_H
#define ARCH4RTOS_SOFTBASEPRI_H

#include  "arch_basetype.h"
#include  "arch4rtos_svc.h"
#include  "CMSIS_Device.h"
#include  "nxRTOSConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/// software extension to implement virtual BASEPRI on CM0,
/// to simplify the use and implementation,
/// the parameter to __set_SOFTBASEPRI  is un-shifted __NVIC_PRIO_BITS
void __set_SOFTBASEPRI(uint32_t basePri);

#ifdef __cplusplus
}
#endif

#endif /* ARCH4RTOS_SOFTBASEPRI_H */

