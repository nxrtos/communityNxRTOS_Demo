/* list_jcb.h
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
#ifndef JCB_LIST_H
#define JCB_LIST_H

//#include  "list_jcb_txt.h"
#include  "arch4rtos.h"
#include  "list.h"
#include  "rtos_jcb.h"

#ifdef __cplusplus
extern "C" {
#endif

extern	JCB_t * pxReadyListJCB; // = NULL;
extern	JCB_t * readyListJCB_Tail; // = NULL;
extern	volatile JCB_t * pxSysDeferListJCB; // = NULL;
extern	volatile JCB_t * pxSysDeferOverflowListJCB; // = NULL;


/// {{{ freeListJCB {{{
JCB_t * xInitfreeListJCB(void);
JCB_t * xappendFreeListJCB(JCB_t * toFreeList);
JCB_t * xpickFreeListJCB(void);
/// }}} freeListJCB }}}

/// {{{ readyListJCB {{{
void       initReadyListJCB(void);
JCB_t * addReadyListJCB(JCB_t * toReadyList);
JCB_t * pickReadyListJCB(JCB_t * pickJCB);
JCB_t * pickHeadReadyListJCB(void);
/// }}} readyListJCB }}}

/// {{{ run2TermListJCB {{{
JCB_t * pushRun2TermListJCB(JCB_t * toList);
JCB_t * popRun2TermListJCB(void);
/// }}} run2TermListJCB }}}

JCB_t * addSysDeferListJCB(JCB_t * pAddJCB, TickType_t  xDeferTicks);
JCB_t * pickSysDeferListJCB(JCB_t * theJCB);


JCB_t * xInitListJCB(void);
#ifdef __cplusplus
}
#endif

#endif	// JCB_LIST_H }}}
