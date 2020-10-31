/* rtos_defer_tcb.h
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
 * DeferTCB is similar to DeferJCB in the management , but the all items
 * for deferring Management is build into BaseTCB_t structure rather than
 * DeferJCB through seperated SoftTimer_ControlBlock binding with JCB.
 *
 * There are pxOsCurrentDeferTCBWaitingList and
 * pxOsOverflowDeferTCBWaitingList to manage the TCB from ready List
 * to WaitingList.
 *
 * As the rule in Run2TermThread_context it is not allowed to invoke
 * rtosDelay(ticks).  Only in Run2BlckThread_context allowed to call
 * rtosDelay(ticks).
 *---------------------------------------------------------------------------*/
////    rtos_defer_tcb.h
/// {{{
#ifndef     RTOS_DEFER_TCB_H
#define    RTOS_DEFER_TCB_H

#include    "rtos_tcb.h"

extern  volatile R2BTCB_t * pxDeferTCBCurrentWaitingList;   // = NULL;
extern  volatile R2BTCB_t * pxDeferTCBOverflowWaitingList;  // = NULL;

#ifdef __cplusplus
extern "C" {
#endif

R2BTCB_t *  pickFromDeferTCBWaitingList(R2BTCB_t *  theR2BTCB );
R2BTCB_t *  addToDeferTCBWaitingList(R2BTCB_t *  theR2BTCB );

void vThreadDelay( const TickType_t xTicksToDelay );
#ifdef __cplusplus
}
#endif

#endif      ///         RTOS_DEFER_TCB_H }}}
