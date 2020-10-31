/* arch4rtos_ipl_mask.h
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 *Interrupt Priority Level Mask, support for CortexM4
 * 1 tab == 4 spaces!
 */


/*-----------------------------------------------------------------------------
 * Implementation of functions for Interrupt Request Priority Base Mask
 * on ARM CM4F.
 * This is more or less the wrapper to manipulation of  PRIMASK and BASEPRI
 * on ARM CM4F.  As the ARM_CM4/F support multiple priority on IRQ/ISR,
 * it is possible a higher priority of ISR to preempt a lower priority of ISR
 * in the middle of service.  This architecture's feature could be great help
 * to more precisely assign priority to different handler and it also could
 * leveraged for RTOS implementation to perform all RTOS kernel activities
 * under certain of IRQ PriorityLevel and leave higher PriorityLevel of IRQ
 * not to be impacted by RTOS kernel to defer response to action.
 *
 * In ISR_context, the current ISR automatic suppress all IRQ with same
 * priority or lower. So a little more complicated abstraction of
 * SysCriticalLevel may used for the this purpose and use it as lower
 * supporting layer or replace it.
 *
 * Interrupt request Priority Base Level Mask,
 *---------------------------------------------------------------------------*/

#ifndef     ARCH4RTOS_IPL_MASK_H    ///{{{
#define    ARCH4RTOS_IPL_MASK_H

#include "arch4rtos_basedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern  IrqPriorityBase_t   xGetIrqPriorityBase(void);
extern  IrqPriorityBase_t   xSetIrqPriorityBase(IrqPriorityBase_t newBase);
extern  IrqPriorityBase_t   xRaiseIrqPriorityBase(IrqPriorityBase_t newBase);
extern  IrqPriorityBase_t   xDropIrqPriorityBase(IrqPriorityBase_t newBase);


#ifdef __cplusplus
}
#endif

#endif  /*ARCH4RTOS_IPL_MASK_H  *////}}}
