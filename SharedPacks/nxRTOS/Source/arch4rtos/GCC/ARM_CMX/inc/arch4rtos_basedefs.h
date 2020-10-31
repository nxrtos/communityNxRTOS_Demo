/* arch4rtos_basedefs.h
 * nxRTOS Kernel V0.0.1
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
 * 1 tab == 4 spaces!
 */


#ifndef     ARCH4RTOS_BASEDEFS_H
#define    ARCH4RTOS_BASEDEFS_H

#include    "arch_basetype.h"
#include    "CMSIS_Device.h"

#define WAIT_FOREVER                                ((TickType_t) -1)

#define	IrqPriorityMaskLevelHIGHEST		(0)
#define	IrqPriorityMaskLevelLOWEST		((0x01 << __NVIC_PRIO_BITS) -1)
#define	IrqPriorityMaskLevelNOMASK		(0x01 << __NVIC_PRIO_BITS)

//  32bit, 4 byte stack alignment
///	StackType_t is defined as typedef in "arch_basetype.h"
/// and (1 << STACK_ALIGNMENT_BITS)  should be sizeof(StackType_t)
#define  STACK_ALIGNMENT_BITS					2
#define  STACK_GROW_TOLOWADDR					1

#define  RTOS_INITIAL_RUN2TERM_SP               (*((StackType_t *)(0)))

/// TODO  rename  and fix
#define     portBYTE_ALIGNMENT                          32
#if portBYTE_ALIGNMENT == 32
    #define portBYTE_ALIGNMENT_MASK ( 0x001f )
#endif


#ifdef __cplusplus
extern "C" {
#endif
///{{{

///}}}
#ifdef __cplusplus
}
#endif

#endif /* ARCH4RTOS_BASEDEFS_H */

