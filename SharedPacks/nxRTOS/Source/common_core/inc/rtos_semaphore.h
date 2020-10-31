/* rtos_semaphore.h
 *
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
 * Implementation of functions for job control
 *---------------------------------------------------------------------------*/
// {{{
#ifndef RTOS_SEMAPHORE_H
#define RTOS_SEMAPHORE_H

#include  "arch4rtos_basedefs.h"
#include  "list.h"
#include  "rtos_xcb_base.h"
#include  "rtos_jcb.h"
#include  "rtos_tcb.h"

typedef     struct      _semaphore_struct
{
    ListXItem_t    * pXItem;    // this pointer to Item in freeSemList
                                                    //  or in acquiringR2BlckTCB
    uint16_t            tokens;  ///< Current number of tokens
    uint16_t            max_tokens;  ///< Maximum number of tokens

    XCB_Base_t    * pxWaitingJobList; // point to first JCB wait for the Sema
} Sem_t;

#ifdef __cplusplus
extern "C" {
#endif

int     initSysSemFreeList(void);
Sem_t     * xSemNew(uint32_t max_count,  uint32_t initial_count);
int             xSemDelete(Sem_t * theSem);
int             xSemRelease( Sem_t * theSem);
int             xSemGetCount(Sem_t * theSem);
Sem_t * pxSemAcquire(Sem_t * theSem, TickType_t timeout);

int    pickTCBFromSemWaitingList(Sem_t * theSem, R2BTCB_t * theTCB);
int    addJCBToSemWaitingList(Sem_t * theSem, JCB_t * theJCB);
#ifdef __cplusplus
}
#endif

#endif	// RTOS_SEMAPHORE_H }}}
