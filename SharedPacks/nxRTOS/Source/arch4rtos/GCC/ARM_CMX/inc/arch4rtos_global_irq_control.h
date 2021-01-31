/**
  ******************************************************************************
  * @file           : arch4rtos_global_irq_control.h
  * @brief          : in progress of ToDo List
  ******************************************************************************
  * @attention      nxRTOS Kernel V0.0.1
 *              Copyright (C) 2019 . or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */

#ifndef  ARCH4RTOS_GLOBAL_IRQ_CONTROL_H   //{{{
#define  ARCH4RTOS_GLOBAL_IRQ_CONTROL_H   //{{{

#if  0
#include  "arch4rtos_global_irq_control.txt.h"
#endif

#include  "arch_basetype.h"
#include  "CMSIS_Device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef   uint32_t  SysGlobalIRQMask_t;
//  this is essential GlobalIRQMask
#define  SysGlobalIRQ_Unmask    (SysGlobalIRQMask_t)0
#define  SysGlobalIRQ_Masked    (SysGlobalIRQMask_t)1
#define  SysGlobalIRQ_Enabled   (SysGlobalIRQ_Unmask)
#define  SysGlobalIRQ_Disabled  (SysGlobalIRQ_Masked)

__STATIC_FORCEINLINE
SysGlobalIRQMask_t arch4rtos_iGetGlobalIRQMask(void)
{
  return  __get_PRIMASK();
}
__STATIC_FORCEINLINE
void arch4rtos_iSetGlobalIRQMask(SysGlobalIRQMask_t newMask)
{
  __set_PRIMASK(newMask);
}

#ifdef __cplusplus
}
#endif

#endif //}}} ARCH4RTOS_GLOBAL_IRQ_CONTROL_H }}}

