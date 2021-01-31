/* pick_tcb.h
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
 * pickTCB() will try to find and return the Thread with highest priority
 * at the moment in the RTOS system, update it to getCurrentTCB().
 * it look between readyJobList, readyThreadList to find the
 * highest priority of Thread to serve to.
 * for performance concern,  a 2-wayLinkList for each prioritizedReadyJobList
 * and prioritizedRunningThreadList.
 *  There is a runningToTermiThreadList to kept all runningToTermiThread
 *  in List and ordered by preemption and stacked.
 *---------------------------------------------------------------------------*/

#ifndef	PICK_TCB_H
#define PICK_TCB_H

#include "rtos_tcb_live.h"

LiveTCB_t * pickTCB(LiveTCB_t * thisTCB);

#endif
