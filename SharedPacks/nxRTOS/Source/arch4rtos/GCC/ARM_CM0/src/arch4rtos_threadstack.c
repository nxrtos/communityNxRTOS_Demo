/* arch4rtos_threadstack.c
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

#include    "arch4rtos_threadstack.h"
#include    "rtos_unified_jobhandler.h"

/**
  * @brief This function handles Pendable request for system service.
  * refer :::
  * 	void xPortPendSVHandler( void )  in port.c in FreeRTOS
  *		PendSV_Handler:					 in os_pendsv_handler.s in os.h
  *		void PendSV_Handler(void) 		 in qxk_port.c in QXK
  */

/*
Exception frame saved by the NVIC hardware onto stack:
+------+
|      | <- SP before interrupt (orig. SP)
| xPSR |
|  PC  |
|  LR  |
|  R12 |
|  R3  |
|  R2  |
|  R1  |
|  R0  | <- SP after entering interrupt (orig. SP + 32 bytes)
+------+

Registers saved by the software (PendSV_Handler):
| EXC_RETURN_value in LR
+------+
|  R7  |
|  R6  |
|  R5  |
|  R4  |
|  R11 |
|  R10 |
|  R9  |
|  R8  | <- Saved SP (orig. SP + 64 + 4 bytes)
+------+
*/

/**
 *
 *<pre>
 StackType_t *arch4rtos_pInitThreadStack( StackType_t *pxTopOfStack, JCB_t *pvParameters );</pre>
 *
 * prepare virgin thread_context in thread_stack for a new born thread from readyJob.
 *
 *
 * @param pxTopOfStack pointer to the location where thread_stack started.
 *
 * @param pvParameters A pointer to JobControlBlock.
 *
 * Example usage:
   <pre>
*/

StackType_t *arch4rtos_pInitThreadStack( StackType_t *pxTopOfStack, JCB_t *pTheJCB )
{
    /* fill-in the thread_to_isr_stack frame as it would be created by a context switch
    interrupt. */
    pxTopOfStack--; /* Offset added to account for the way the MCU uses the stack on entry/exit of interrupts. */
    // fabric xPSR to ( 0x01000000 ) for CM0. clear all but set EPSR
    // EPSR[24] Thumb state bit = 1;  APSR = 0; IPSR = 0
    // for the APSR may orr with it.
    *pxTopOfStack = ( 0x01000000 ); // portINITIAL_XPSR;   /* xPSR */
    pxTopOfStack--;
    // PC_value in  bthread_to_isr_stack frame expected bit[0] clear
    *pxTopOfStack = (StackType_t)unified_Job_Handler & 0xFFFFFFFE; /* PC */
    pxTopOfStack--;
    // this can be a error handler for unexpected return from unified_Job_Handler.
    // As the function pointer has automatic treated with bit[0] set.
    *pxTopOfStack = ( StackType_t ) unified_Job_Handler;    /* LR */
    pxTopOfStack -= 5;  /* R12, R3, R2 and R1. */
    *pxTopOfStack = ( StackType_t ) pTheJCB;   /* R0 */

    pxTopOfStack--;     // 0xFFFFFFFD-Thread Stack uses PSP
    *pxTopOfStack = ( 0xFFFFFFFD ); //  EXC_RETURN_value (1 reg size) */

    pxTopOfStack -= 8; /* R11...R4 (8 regs). */

    return pxTopOfStack;
}
