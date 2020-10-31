/* rtos_stackspace_isr.c
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

#include  "rtos_stackspace.h"
#include  "arch4rtos_basedefs.h"
#include  "nxRTOSConfig.h"

/// {{{ StackSpace_ISR  {{{
#define	ISR_StackSPAC_SizeInStackType_t				\
				((RTOS_ISR_Stack_SIZE + sizeof(StackType_t) -1) 	\
										/(sizeof(StackType_t)))

static StackType_t      theISR_StackSPACE[ISR_StackSPAC_SizeInStackType_t];

#if     defined(STACK_GROW_TOUPADDR)
    #undefine   STACK_GROW_TOLOWADDR
    const   StackType_t  *  INITIAL_ISR_StackPoint  =
                                &theISR_StackSPACE[0];
#else
    #define     STACK_GROW_TOLOWADDR    1
    const   StackType_t  *  INITIAL_ISR_StackPoint  =
                &theISR_StackSPACE[ISR_StackSPAC_SizeInStackType_t -1];
#endif

static StackType_t     xDetect_ISR_Stack_Overflow_Configured = 0;

void    setDetection_ISR_Stack_Overflow()
{
#if    defined(STACK_GROW_TOLOWADDR)
    theISR_StackSPACE[0] = 0x5aa5effe;
#else
    theISR_StackSPACE[ISR_StackSPAC_SizeInStackType_t -1] = 0x5aa5effe;
#endif

    xDetect_ISR_Stack_Overflow_Configured = 1;
}

void    checkDetection_ISR_Stack_Overflow()
{
    if(xDetect_ISR_Stack_Overflow_Configured)
    {
#if    defined(STACK_GROW_TOLOWADDR)
      if(theISR_StackSPACE[0] != 0x5aa5effe)
#else
      if(theISR_StackSPACE[ISR_StackSPAC_SizeInStackType_t -1] != 0x5aa5effe)
#endif
      {
    	  while(1);
      }
    }
}

/// }}} StackSpace_ISR  }}}
