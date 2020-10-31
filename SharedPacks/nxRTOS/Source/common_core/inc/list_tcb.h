/* list_tcb.h
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
#ifndef TCB_LIST_H
#define TCB_LIST_H

#include  "arch4rtos.h"
#include  "list.h"
#include  "rtos_tcb.h"

extern	BaseTCB_t * pxCurrentTCB; //= NULL;
extern	BaseTCB_t * pxRun2TermTCBList_Head; //= NULL;
extern	BaseTCB_t * pxRun2BlckTCBList_Head; // = NULL;

/// {{{ run2TermListTCB {{{
BaseTCB_t * pushRun2TermListTCB(BaseTCB_t * toList);
BaseTCB_t * popRun2TermListTCB(void);
/// }}} run2TermListTCB }}}


BaseTCB_t * addTCBToRun2BlckTCBList(BaseTCB_t * theTCB);
BaseTCB_t * pickTCBFromRun2BlckTCBList(BaseTCB_t * theTCB);
BaseTCB_t * getCurrentTCB(void);

#endif	// TCB_LIST_H }}}
