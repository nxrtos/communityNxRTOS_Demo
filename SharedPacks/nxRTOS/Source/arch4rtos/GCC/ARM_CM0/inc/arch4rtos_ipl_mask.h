/* arch4rtos_ipl_mask.h
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
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


/*-----------------------------------------------------------------------------
 * Implementation of functions for Interrupt Request Priority Base Mask
 * on ARM CM0
 * Because the ARM_CM0 has no BASEPRI implemented on the architecture,
 * the IrqMask features completely rely on   PRIMASK, which relegate to
 * only IrqEnable and IrqDisable.
 * *
 * In ISR_context, the current ISR automatic suppress all IRQ with same
 * priority or lower. So a little more complicated abstraction of
 * SysCriticalLevel may used for the this purpose and use it as lower
 * supporting layer or replace it.
 *
 * Interrupt request Priority Base Level Mask,
 * This is less useful on ARM_CM0 than on ARM_CM4/F
 *---------------------------------------------------------------------------*/

#ifndef     ARCH4RTOS_IPL_MASK_H    ///{{{
#define    ARCH4RTOS_IPL_MASK_H

#include "arch4rtos_basedefs.h"
#include "nxRTOSConfig.h"

//#define     configIRQ_PRIORITY_BASE_MASK_SUPPORT
///     {{{
#ifdef        configIRQ_PRIORITY_BASE_MASK_SUPPORT

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

#endif      ///    configIRQ_PRIORITY_BASE_MASK_SUPPORT      }}}
#endif  /*ARCH4RTOS_IPL_MASK_H  *////}}}
