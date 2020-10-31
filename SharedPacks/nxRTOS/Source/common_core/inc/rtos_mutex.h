/* rtos_mutex.h
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
#ifndef RTOS_MUTEX_H
#define RTOS_MUTEX_H

#include  "arch4rtos_basedefs.h"
#include  "list.h"
#include  "rtos_jcb.h"
#include  "rtos_tcb.h"

 struct      _mutex_struct;

typedef     struct      _mutex_struct
{
    ListXItem_t    * pXItem;    // this pointer to Item in freeSemList
                                                    //  or point to MutexR2BlckThreadWaitingList
    JCB_t                * job_waiting_list;
    JCB_t                * owner_job;
    R2BTCB_t        * owner_thread;  ///< Owner Thread
    struct              _mutex_struct   *   pxNextMutex;
} Mutex_t;

#ifdef __cplusplus
extern "C" {
#endif

int     initSysMutexFreeList(void);
Mutex_t  * xMutexNew(Mutex_t *);
int             xMutexDelete(Mutex_t * theMuex);
int             xMutexRelease( Mutex_t * theMuex);
void *       xMutexGetOwner(Mutex_t * theMuex);
void *       xMutexGetWaiter(Mutex_t * theMuex);
Mutex_t *   pxThreadMutexAcquire(Mutex_t * theMuex, TickType_t timeout);
Mutex_t *   pxJobMutexAcquire(Mutex_t * theMuex, JCB_t * theJCB);

int    pickTCBFromMutexWaitingList(Mutex_t * theMutex, R2BTCB_t * theTCB);
int    cleanAllMutexFromHoldingList(R2BTCB_t * theTCB);

int    addJCBToMutexWaitingList(Mutex_t * theMutex, JCB_t * theJCB);
int    pickJCBFromMutexWaitingList(Mutex_t * theMutex, JCB_t * theJCB);

#ifdef __cplusplus
}
#endif

#endif	// RTOS_MUTEX_H }}}
