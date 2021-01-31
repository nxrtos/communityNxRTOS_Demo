/* arch4rtos_pendsv.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */


/*-----------------------------------------------------------------------------
 * Implementation of functions defined in arch4rtos.h for the architecture of
 * ARM CM4F.
 * Refer to arch4rtos.tx for design consideration.
 *---------------------------------------------------------------------------*/
#include "arch4rtos.h"
#include "pick_tcb.h"
/**
  * @brief This function handles Pendable request for system service.
  * refer :::
  *   void xPortPendSVHandler( void )  in port.c in FreeRTOS
  *   PendSV_Handler:         in os_pendsv_handler.s in os.h
  *   void PendSV_Handler(void)         in qxk_port.c in QXK
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


void PendSV_Handler(void)	 __attribute__ (( naked ));

void PendSV_Handler(void)
{

  // TODO , debug trace hard fault, enable before exit
  __disable_irq();
  // any to do with R0 to R3 as they had pushed into sys stack
  __asm volatile
  ( //  assume Thread mode with PSP, in case of MSP fix it later.
    "   mrs     r0, PSP                             \n"
    "   mov     r2, lr                              \n"
    //  Write value of 0x4 to R3, flags get up
    "   movs    r3,#4                              \n"
    //The TST instruction performs a bitwise AND operation
    //  on the value in R2 and the value in R3
    "   TST     r2, r3                              \n"
    // NE means EXC_RETURN value =0xFFFFFFFD, aka. Thread SP uses PSP
    "   BNE label_done_thread_sp_in_r0          \n"
    /* assume EXC_RETURN value =0xFFFFFFF9-Thread Stack uses MSP */
    "   mrs     r0, msp                         \n"
    "   mov     r1, r0                          \n"
    "   sub     r1, r1, #40                     \n"
    // adjust msp for the space to save 8 registers and lr
    // if more to save, adjust the size
    "   msr	    msp, r1                         \n"
    "   movs    r3,#8                           \n"
    "   TST     r2, r3                          \n"
    // NE means EXC_RETURN value =0xFFFFFFF9, aka. Thread SP uses MSP
    "   BNE label_done_thread_sp_in_r0          \n"
    /* EXC_RETURN value =0xFFFFFFF1, Handler Stack uses MSP */
    //  otherwise, something wrong here,
    //  this is in PendSV, must be lowest ISR priority and
    //  can't be preempt other ISR, so can't be
    //  panic here!
    "   label_done_thread_sp_in_r0 :        \n"
    "                                       \n"
  );

#if		0
///{{{
    // LDM Rn{!}, reglist
    // STM Rn!, reglist

    // LDM instructions load the registers in reglist with word values from
    // memory addresses based on Rn.
    // STM instructions store the word values in the registers in reglist to
    // memory addresses based on Rn.
    //  !    Writeback suffix.

    // LDMIA and LDMFD are synonyms for LDM. LDMIA refers to the base register
    // being Incremented After each access. LDMFD refers to its use for popping
    // data from Full Descending stacks.
    // STMIA and STMEA are synonyms for STM. STMIA refers to the base register
    // being Incremented After each access. STMEA refers to its use for pushing
    // data onto Empty Ascending stacks
///}}}
#endif

  __asm volatile
  ( //	push lr into thread_stack
    "   sub     r0, r0, #4                  \n"
    ///* New thread_SP point A */
    "   stmia   r0!,{r2}                    \n"
    "   sub     r0, r0, #4                  \n"
    ///* get back to thread_SP point A */
  );

  __asm volatile
  ( //	push r4-r11 into thread_stack, total 8 registers
    "   sub     r0, r0, #16                 \n"
    "   stmia   r0!,{r4-r7}                 \n"
    ///* Make thread_SP point B */
    "   mov     r4, r8                      \n"
    "   mov     r5, r9                      \n"
    "   mov     r6, r10                     \n"
    "   mov     r7, r11                     \n"
    "   sub     r0, r0, #32                 \n"
    // 16 /* get back to thread_SP point B */
    // + 16 /* New thread_SP point C */
    "   stmia   r0!,{r4-r7}                 \n"
    "   sub     r0, r0, #16                 \n"
    // 16 /* get back to thread_SP point C */
  );

  // {{{ raise PendSV ISR priority level, {{{
#if (RTOS_SYSCRITICALLEVEL)
  __asm volatile
  (
    " push  {r0}     \n"
    " push  {r1}     \n"
    " push  {r2}     \n"
    " push  {r3}     \n"

    " mov   r3, LR    \n"
    " push  {r3}     \n"
  );
  __NVIC_SetPriority(PendSV_IRQn, RTOS_SYSCRITICALLEVEL);
  __asm volatile
  (
    " pop  {r3}      \n"
    " mov  LR, r3    \n"

    " pop  {r3}     \n"
    " pop  {r2}     \n"
    " pop  {r1}     \n"
    " pop  {r0}     \n"
  );

#else
  __disable_irq();
#endif

  //xSetContextPriorityLevel(NULL, IrqPriorityMaskLevelHIGHEST);
  //NVIC_SetPriority(PendSV_IRQn, 0); // 0 is highest

  __asm volatile
  ( //
    "   ldr     r3, pxCurrentLiveTCBConst           \n"
    /* Get the location of the current TCB. */
    "   ldr     r2, [r3]                        \n"
    /* now r2 have the pointer to current TCB */
    "   str     r0, [r2]                        \n"
    /* Save the new top of stack. */
    "   mov     r0, r2                          \n"
  );

  // clear PendSV, use r1 to r3; This operation has to be within the critical section
  SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk;

  // get the target ThreadControlBlock will the Thread scheduled.
  // switching critical

    // restore PendSV ISR priority level to lowest
    //xSetContextPriorityLevel(NULL, IrqPriorityMaskLevelLOWEST);
    //NVIC_SetPriority(PendSV_IRQn, IrqPriorityMaskLevelLOWEST); //

    // restore the latest Thread_SP value from ThreadControlBlock into r0
    __asm volatile
    (
        "   bl      pickTCB                     \n"
        // /* now r0 have the pointer to current TCB */
        "   ldr     r0, [r0]                    \n"
        // get the new Thread_SP in r0
    );

    __asm volatile
    (	//	pop r4-r11 from thread_stack, total 8 registers
        //  r0 holds the stack_pointer of the thread_stack
        "   ldmia   r0!,{r4-r7}                 \n"
        "   mov     r8, r4                      \n"
        "   mov     r9, r5                      \n"
        "   mov     r10, r6                     \n"
        "   mov     r11, r7                     \n"
        "   ldmia   r0!,{r4-r7}                 \n"
    );

    __asm volatile
    (	//	pop lr from thread_stack
        "   ldmia   r0!,{r2}                    \n"
        "   mov     lr, r2                      \n"
    );

    __asm volatile
    (   /* EXC_RETURN value =0xFFFFFFF9-Thread Stack uses MSP */
        "   ldr     r3, =0xFFFFFFF9                 \n"
        "   cmp     r2, r3                      	\n"
        "   bne     label_no_msp_adjust             \n"
        "   msr     msp, r0                         \n"
        "label_no_msp_adjust: nop                   \n"
    );

    __asm volatile
    (   /* EXC_RETURN value =0xFFFFFFFD-Thread Stack uses PSP */
        "   ldr     r3, =0xFFFFFFFD                 \n"
        "   cmp     r2, r3                          \n"
        "   bne     label_no_psp_adjust             \n"
        "   msr     psp, r0                         \n"
        "label_no_psp_adjust: nop                   \n"
    );

    // }}} drop PendSV priority level back to lowest }}}
    // it has to enable global irq before return from PenSV
#if (RTOS_SYSCRITICALLEVEL)
    __asm volatile
    (
      " mov  r3, LR    \n"
      " push  {r3}; \n"
    );
  ARCH_NVIC_SetPriority(PendSV_IRQn, LOWEST_ISRCRITICALLEVEL);
  __asm volatile
  (
      " pop  {r3};  \n"
      " mov  LR,  r3\n"

  );
#endif
    __enable_irq();

    __asm volatile
    (   // regular return;
        "   bx  LR                      \n"
        "   .align 4                    \n"
        "pxCurrentLiveTCBConst: .word pxCurrentLiveTCB      \n"
    );
}
