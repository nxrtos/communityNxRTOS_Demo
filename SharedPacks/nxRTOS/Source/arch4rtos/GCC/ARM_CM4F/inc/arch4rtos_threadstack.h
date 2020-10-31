/* arch4rtos_threadstack.h
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

#ifndef     ARCH4RTOS_THREADSTACK_H
#define     ARCH4RTOS_THREADSTACK_H

#include  "arch4rtos_basedefs.h"
#include  "rtos_jcb.h"

#ifdef __cplusplus
extern "C" {
#endif



/**
 * StackType_t *arch4rtos_pInitThreadStack
 *              ( StackType_t *pxTopOfStack, JCB_t *pvParameters );
 *
 * prepare virgin thread_context in thread_stack for a new born thread from
 * readyJob.
 *
 * @param pxTopOfStack pointer to the location where thread_stack started.
 *
 * @param pvParameters A pointer to JobControlBlock.
 *
 * Example usage:
*/

StackType_t *arch4rtos_pInitThreadStack
                               ( StackType_t *pxTopOfStack, JCB_t *pReadyJCB );
unsigned long getInitialSPSize4Run2TermThread(void);
StackType_t * getInitialSPSetting4Run2TermThread(void);

#ifdef __cplusplus
}
#endif

#endif  //  ARCH4RTOS_THREADSTACK_H
