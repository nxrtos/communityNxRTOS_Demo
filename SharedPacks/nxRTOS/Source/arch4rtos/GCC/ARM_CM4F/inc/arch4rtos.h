/*
 * nxRTOS Kernel V10.0.1
 * Copyright (C) 2019 . or its affiliates.  All Rights Reserved.
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
 * arch4rtos.h  for  GCC/ARM_CM4F/
 */


#ifndef ARCH4RTOS_H
#define ARCH4RTOS_H

#include "arch_basetype.h"
#include "arch4rtos_ipl_mask.h"
#include "CMSIS_Device.h"

///{{{  redefine here for eclipse , no real need as they are from CMSIS defs
#ifndef  CORTEX_M
#define  CORTEX_M     4U
#endif
#ifndef  __ARM_ARCH_7EM__
#define  __ARM_ARCH_7EM__     1U
#endif
///}}}


typedef uint32_t SysIRQ_Type;
// common exceptions
#define	SysIRQ_NMI					2	/*	(NonMaskableInt_IRQn +16) Non Maskable Interrupt                                          */
#define	SysIRQ_MemoryManagement     4 	/*Cortex-M4 Memory Management Interrupt                           */
#define	SysIRQ_BusFault             5 	/*Cortex-M4 Bus Fault Interrupt                                   */
#define	SysIRQ_UsageFault           6 	/*Cortex-M4 Usage Fault Interrupt                                 */
#define	SysIRQ_SVCall               11 	/*Cortex-M4 SV Call Interrupt                                    */
#define	SysIRQ_DebugMonitor         12 	/*Cortex-M4 Debug Monitor Interrupt                              */
#define	SysIRQ_PendSV               14 	/*Cortex-M4 Pend SV Interrupt                                    */
#define	SysIRQ_SysTick              15 	/*Cortex-M4 System Tick Interrupt                                */

#ifdef __cplusplus
extern "C" {
#endif
///{{{	PriorityMaskLevel manipulate

extern uint32_t xGetThreadPriorityMaskLevel(void);
extern uint32_t xSetThreadPriorityMaskLevel(uint32_t newLevel);

extern uint32_t xGetOsPriorityMaskLevel(void);
extern uint32_t xSetOsPriorityMaskLevel(uint32_t newLevel);
///}}}

///{{{	ContextPriorityLevel manipulate

extern uint32_t xGetSysIrqPriorityLevel(uint32_t xSysIrq);
extern uint32_t xSetSysIrqPriorityLevel(uint32_t xSysIrq, uint32_t newLevel);

extern uint32_t xGetContextPriorityLevel(void * contextHandle);
extern uint32_t xSetContextPriorityLevel(void * contextHandle, uint32_t newLevel);

///}}}



///{{{  get Current Thread Handle
///TCB_t *	xGetTheadHandle(void);

///}}}
#ifdef __cplusplus
}
#endif

__STATIC_FORCEINLINE    void  arch4rtosReqSchedulerService()
{
  // trigger PendSV
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

__STATIC_FORCEINLINE    void  arch4rtos_thread_termination(void * theTCB)
{
  //void * inR0;
  //(void) theTCB;  // expect in R0
  __asm volatile("    \n");
  //__NVIC_SetPriority(SVCall_IRQn, 0);
  __NVIC_SetPriority(-5, 0);
  __enable_irq();
  __DSB();
  __ISB();
  //    inR0 = theTCB;
  __asm volatile
  (
    "  ldr    r0, [r7, #56] \n" // trick to play to load theTCB to R0
    "  svc    #0xFF   \n"  // SVC_FF_handler()
  );
  __asm volatile
  (
    "  nop    \n"
  );
}

#endif /* ARCH4RTOS_H */

