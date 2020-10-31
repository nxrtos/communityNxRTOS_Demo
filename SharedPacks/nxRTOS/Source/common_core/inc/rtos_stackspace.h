/* rtos_stackspace.h
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

/*
 *  This file defines the stack_space for
 *  ISR_StackSpace, Run2Term_ThreadStackSpace, Run2Blck_ThreadStackSpace
 * */

#ifndef RTOS_STACKSPACE_H
#define RTOS_STACKSPACE_H

#include  "arch_basetype.h"

#ifndef NXRTOS_CONFIG_H
#include    "nxRTOSConfig.h"
#endif

/// {{{ ISR_StackSpace  {{{
extern  const   StackType_t  *  INITIAL_ISR_StackPoint;
/// }}} ISR_StackSpace  }}}

#if     defined(RTOS_INITIAL_RUN2TERM_ThreadSP)
#define     INITIAL_Run2TermThread_StackPoint       RTOS_INITIAL_RUN2TERM_ThreadSP
#else
extern  const   StackType_t  *  INITIAL_Run2TermThread_StackPoint;
#endif


#ifdef __cplusplus
extern "C" {
#endif

void    setDetection_ISR_Stack_Overflow();
void    checkDetection_ISR_Stack_Overflow();

#ifdef __cplusplus
}
#endif

#endif  /// RTOS_STACKSPACE_H
