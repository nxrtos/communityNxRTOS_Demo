/* defer_jcb.h
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


// {{{
#ifndef DEFER_JCB_H
#define DEFER_JCB_H

#include  "rtos_softtimer.h"

// volatile
extern  SoftTimer_t * pxOsCurrentSoftTimerWaitingList; // = NULL;
// volatile
extern  SoftTimer_t * pxOsOverflowSoftTimerWaitingList; // = NULL;

#ifdef __cplusplus
extern "C" {
#endif

extern  SoftTimer_t * pickFromSoftTimerWaitingList(SoftTimer_t * theSoftTimer);
extern  SoftTimer_t * addToSoftTimerWaitingList(SoftTimer_t * theSoftTimer );

#ifdef __cplusplus
}
#endif

#endif  // DEFER_JCB_H
