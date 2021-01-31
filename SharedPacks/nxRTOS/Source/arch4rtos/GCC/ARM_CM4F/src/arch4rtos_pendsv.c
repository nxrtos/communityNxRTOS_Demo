/* arch4rtos_pendsv.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */
/*-----------------------------------------------------------------------------
 * PendSV_Handler for ARM CM4.
 * This folder is for CM4F, but at this moment the implementation is for CM4.
 * Lazy stack for CM4F to be implemented later.
 *---------------------------------------------------------------------------*/
#include "arch4rtos.h"
#include  "pick_tcb.h"
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
|  LR  | // before ISR_Entry with LR_value in Thread_context.
|  R12 |
|  R3  |
|  R2  |
|  R1  |
|  R0  | <- SP_after_entering_interrupt (orig. SP + 32 bytes)
+------+

Registers saved by the software (PendSV_Handler):
+------+
|  LR  | // after ISR_Entry with EXC_RETURN value
|  R11  |
|  R10  |
|  R9  |
|  R8  |
|  R7 |
|  R6 |
|  R5  |
|  R4  | <- Saved SP (orig. SP + 32 + 32 + 4 bytes)
+------+
*/

__attribute__ (( naked ))
void PendSV_Handler(void)
{

  // TODO , debug trace hard fault, enable before exit
  __disable_irq();
  /* This is a naked function to perform transition Thread_context to
   * runningThread with highest priority */
  // disable global irq is a safest way to performing thread context switch
  // but it may overkill if want reserve range of higher ISR_Priority to
  // RTOS_Kernel exclusive and for hard time-critical handling, for example
  // Motor_Control. In those case, raise Exception_PendSV_Priority and restore
  // to Lowest can be proper approach.
  //__disable_irq();
  __asm volatile
  ( //**{{{  load  Thread_SP into R0 {{{
    //  This is implementation to fit either PSP or MSP used for Thread.
    //  It can be optimized if determined to use PSP or MSP for Thread
    //  assume Thread mode with PSP, in case of MSP fix it later.
    " mrs	r0, PSP         \n"
    //  Write value of 0x4 to R3, flags get up
    " movs r3,#4          \n"
    //The TST instruction performs a bitwise AND operation
    //  on the value in R2 and the value in R3
    " TST lr, r3          \n"
    // NE means EXC_RETURN value =0xFFFFFFFD, aka. Thread SP uses PSP
    " BNE label_done_thread_sp_in_r0      \n"

    /* assume EXC_RETURN value =0xFFFFFFF9-Thread Stack uses MSP */
    "   mrs	r0, msp         \n"
    "   mov r1, r0          \n"
    "   sub r1, r1, #40     \n"
    // adjust msp for the space to save 8 registers and lr
    // if more to save, adjust the size
    "   msr	msp, r1         \n"
    "   movs r3,#8          \n"
    "   TST lr, r3          \n"
    // NE means EXC_RETURN value =0xFFFFFFF9, aka. Thread SP uses MSP
    "   BNE label_done_thread_sp_in_r0          \n"
    /* EXC_RETURN value =0xFFFFFFF1, Handler Stack uses MSP */
    //  something wrong here,
    //  this is in PendSV, must be lowest ISR priority and
    //  can't be preempt other ISR, so can't be here
    //  panic for investigation.
    "   label_panic_loop :  bl label_panic_loop \n"
    "   label_done_thread_sp_in_r0 :            \n"
    //**}}}  load  Thread_SP into R0 }}}
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
  ( //	push r4-r11 into thread_stack, total 8 registers,
    // push lr into thread_stack
    " sub   r0,   r0,   #36             \n" // 8 * 4 + 1 * 4
    " stmia r0!,  {r4-r11}              \n"
    " stmia r0!,  {lr}                  \n"
    " sub   r0,   r0,   #36	            \n"// 8 * 4 + 1 * 4
  );

  // preserve the latest Thread_SP value (in r0) into the ThreadControlBlock
  __asm volatile
  ( /* Get the location of the current TCB. */
    " ldr   r3,   pxCurrentLiveTCBConst       \n"
    /* now r2 have the pointer to current TCB */
    " ldr   r2,   [r3]                    \n"
    " cmp   r2,   #0                      \n"
    " BEQ   skip_save_top_of_sp     \n"
    // skip save if pxCurrentLiveTCB == NULL.
    " str   r0,   [r2]                    \n"
    /* Save the new top of stack. */
    " skip_save_top_of_sp:                \n"
    " mov   r0,   r2                      \n"
    // set the first parameter to function of pickTCB
  );

  // shortcut to raise PendSV SysCritical level to RTOS_SYSCRITICALLEVEL
#if (RTOS_SYSCRITICALLEVEL)
  // this function use R4, in case need to preserve, push to stack
  // and then pop after
  __asm volatile
    (
  //    " push  {r4}            \n"
      "                       \n"
    );
  __set_BASEPRI(RTOS_SYSCRITICALLEVEL << (8U - __NVIC_PRIO_BITS));
  __asm volatile
    (
  //    " pop  {r4}            \n"
      "                      \n"
    );

#else
  __disable_irq();
#endif
  //xSetContextPriorityLevel(NULL, IrqPriorityMaskLevelHIGHEST);
  //NVIC_SetPriority(PendSV_IRQn, 0); // 0 is highest

  // clear PendSV, use r1 to r3; This operation has to be within the critical section
  SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk;

  // get the target ThreadControlBlock will the Thread scheduled.
  // switching critical

  // restore PendSV ISR priority level to lowest
  //xSetContextPriorityLevel(NULL, IrqPriorityMaskLevelLOWEST);
  //NVIC_SetPriority(PendSV_IRQn, IrqPriorityMaskLevelLOWEST); //

  // restore the latest Thread_SP value from ThreadControlBlock into r0
  __asm volatile
  ( // some similation code, change registers r0 to r11 etc.
    //  "   pop {r0}                                \n"
    " bl  pickTCB         \n"
    // /* now r0 have the pointer to current TCB */
    " ldr	r0, [r0]        \n" // get the new Thread_SP in r0
  );

  __asm volatile
  ( //  pop r4-r11 from thread_stack, total 8 registers
    //  r0 holds the stack_pointer of the thread_stack
    " ldmia   r0!,{r4-r11}            \n"
  );

  __asm volatile
  ( //  pop  lr from thread_stack
    " ldmia   r0!,{lr}                \n"
  );

  __asm volatile
  ( // assume Thread_context using PSP, but more work to check it up after.
    // preset PSP, it is OK to change PSP if the Thread_context using MSP
    " msr   psp,  r0                  \n"
    //  Write value of 0x4 to R3, flags get up
    " movs  r3,   #4                  \n"
    //The TST instruction performs a bitwise AND operation
    //  on the value in R2 and the value in R3
    " TST   lr,   r3                  \n"
    // NE means EXC_RETURN value =0xFFFFFFFD, aka. Thread SP uses PSP
    " bne   label_no_msp_adjust       \n"
    /* EXC_RETURN value =0xFFFFFFF9-Thread Stack uses MSP */
    " ldr   r3,   =0xFFFFFFF9         \n"
    " cmp   lr,   r3                  \n"
    " bne   label_no_msp_adjust       \n"
    " msr   msp,  r0                  \n"
    " movs  r3,   #8                  \n"
    " TST   lr,   r3                  \n"
    // NE means EXC_RETURN value =0xFFFFFFF1, aka. return to ISR
    // panic.
    " BEQ   label_panic_loop          \n"
    " label_no_msp_adjust:            \n"
  );

  // drop PendSV SysCriticalLevel back to lowest ISR_SysCriticalLevel
#if (RTOS_SYSCRITICALLEVEL)
  __set_BASEPRI(0);
#endif
  // it has to enable global irq before return from PenSV
  __enable_irq();

  __ISB();
  __DSB();
  __asm volatile
  ( // regular return;
    " bx    LR                    \n"
    " .align 4                    \n"
    "pxCurrentLiveTCBConst: .word pxCurrentLiveTCB  "
  );
}
