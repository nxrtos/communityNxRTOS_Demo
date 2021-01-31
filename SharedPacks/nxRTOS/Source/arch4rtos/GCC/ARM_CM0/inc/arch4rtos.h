/*
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019 . or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */

#ifndef ARCH4RTOS_H
#define ARCH4RTOS_H

#include  "arch4rtos_basedefs.h"
#include  "CMSIS_Device.h"

///{{{  redefine here for eclipse , no real need as they are from CMSIS defs
#ifndef  CORTEX_M
#define  CORTEX_M     0U
#endif
#ifndef  __ARM_ARCH_6M__
#define  __ARM_ARCH_6M__     1U
#endif
///}}}

#define  IrqPriorityMaskLevelHIGHEST  (0)
#define  IrqPriorityMaskLevelLOWEST   ((0x01 << __NVIC_PRIO_BITS) -1)
#define  IrqPriorityMaskLevelNOMASK   (0x01 << __NVIC_PRIO_BITS)


#ifdef __cplusplus
extern "C" {
#endif

extern
void ARCH_NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority);

///{{{  PriorityMaskLevel manipulate
extern
uint32_t  xGetThreadPriorityMaskLevel(void);
extern
uint32_t  xSetThreadPriorityMaskLevel(uint32_t newLevel);
extern
uint32_t  xGetSysPriorityLevel(void);
extern
uint32_t  xSetSysPriorityLevel(uint32_t newLevel);
///}}}

///{{{	ContextPriorityLevel manipulate
extern
uint32_t  xGetContextPriorityLevel(void * contextHandle);
extern
uint32_t  xSetContextPriorityLevel(void * contextHandle, uint32_t newLevel);
///}}}
#ifdef __cplusplus
}
#endif

__STATIC_FORCEINLINE    void  arch4rtosReqSchedulerService()
{
    // trigger PendSV
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

//__STATIC_FORCEINLINE
extern
void  arch4rtos_thread_termination(void * theTCB);

#endif /* ARCH4RTOS_H */
