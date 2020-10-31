/* arch4rtos_svc.c
 *
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 2 spaces!
 */


/*-----------------------------------------------------------------------------
 * Implementation of SVC_Handler  for ARM_CM0.
 *
 *---------------------------------------------------------------------------*/

#include  "arch4rtos_svc.h"
#include  "arch4rtos_criticallevel.h"
#include  "arch4rtos_testcriticallevel.h"
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
|  R0  | <- SP_after_entering_interrupt (orig. SP + 32 bytes)
+------+  may need to  re-save  in the software extension to
              manipulate SysCriticalLevel from Thread_context.
*/

/*
 * EXC_RETURN value Description
 *
 * 0xFFFFFFF1 Return to Handler mode. 	(-15, -0xf),  1111...11110001
 * Exception return gets state from the main stack.
 * Execution uses MSP after return.
 *
 * 0xFFFFFFF9 Return to Thread mode.	(-7), 1111...11111001
 * Exception return gets state from MSP.
 * Execution uses MSP after return.
 *
 * 0xFFFFFFFD Return to Thread mode.	(-3), 1111...11111101
 * Exception return gets state from PSP.
 * Execution uses PSP after return.
 *
 */

void SVC_Handler_apphook(void);
//static
void ARCH_NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority);

#if	01
/**
  * @brief This function handles System service call via SWI instruction.
  */
__attribute__ (( naked ))
void SVC_Handler(void)
{
  // {{{ Cortex-M0 means (__CORTEX_M == 0), and so (__ARM_ARCH_6M__ == 1) {{{
#if (defined (__ARM_ARCH_6M__) && (__ARM_ARCH_6M__ == 1))
  __asm volatile
  (
    //  assume Thread mode with PSP, in case of MSP fix it later.
    " mrs   r0, PSP                     \n"
    " mov   r2, lr                      \n"
    //  Write value of 0x4 to R3, flags get up
    " movs  r3, #4                      \n"
    //The TST instruction performs a bitwise AND operation
    //  on the value in R2 and the value in R3
    " TST   r2, r3                      \n"
    // NE means EXC_RETURN value =0xFFFFFFFD, aka. Thread SP uses PSP
    " BNE   label_done_thread_sp_in_r0  \n"
    " mrs   r0, msp                     \n"
    //  if we allow "svc #num" in ISR handler,  find it by
    //  movs r3,#8 ; TST r2, r3
    " label_done_thread_sp_in_r0 :      \n"
  );
#endif
  // }}} Cortex-M0 means (__CORTEX_M == 0), and so (__ARM_ARCH_6M__ == 1) }}}

  // {{{ Cortex-M4 means (__CORTEX_M == 4), and so (__ARM_ARCH_7EM__ == 1) {{{
#if (defined (__ARM_ARCH_7EM__     ) && (__ARM_ARCH_7EM__     == 1))
  __asm volatile(
    "TST lr, #4     \n"
    "ITE EQ         \n"
    "MRSEQ r0, MSP  \n"
    "MRSNE r0, PSP  \n"
  ) ;
  __asm volatile
  (
    " mov   r2,   lr    \n"
  );
#endif
  // }}} Cortex-M4 means (__CORTEX_M == 4), and so (__ARM_ARCH_7EM__ == 1) }}}

  __asm volatile
  (
    // svc_number = ((char *)svc_args[6])[-2];
    " ldr   r3,   [r0, #24]       \n"
    " sub   r3,   #2              \n"
    // switch(svc_number)
    " ldrb  r3,   [r3, #0]        \n"
    // case ff
    " cmp   r3,   #0xff           \n"
    " beq   svc_ff                \n"
    // case 00:
    " cmp   r3,   #0              \n"
    " beq   SVC_00_handler        \n"
    " cmp   r3,   #1              \n"
    " beq   svc_01                \n"     //    SVC_01_handler      \n"
    " cmp   r3,   #6              \n"
    " beq   svc_06                \n"
    " cmp   r3,   #7              \n"
    " beq   svc_07                \n"
    " cmp   r3,   #8              \n"
    " beq   svc_08                \n"
    " cmp   r3,   #254            \n"
    " beq   svc_FE                \n"
    // default:
    " svc_00:                     \n"
    // r2 hold EXC_RETURN value at this moment
    " bl    SVC_00_handler        \n"
    " bx    r2                    \n"
    " svc_01:                     \n"
    " bl    SVC_01_handler        \n"
    " bx    lr                    \n"
    " svc_06:                     \n"
    // r2 hold EXC_RETURN value at this moment
    " bl    SVC_06_handler        \n"
    " svc_07:                     \n"
    // r2 hold EXC_RETURN value at this moment
    " bl    SVC_07_handler        \n"
    // catch test no return
    " b     loop4ever             \n"
    " svc_08:                     \n"
    // r2 hold EXC_RETURN value at this moment
    " push  {r2}                  \n"
    " bl    SVC_08_handler        \n"
    //  design could be no return
    " pop   {r2}                  \n"
    // regular return;
    " bx    r2                    \n"
    " svc_ff:                     \n"
    // r2 hold EXC_RETURN value at this moment
    " bl    SVC_FF_handler        \n"
    " svc_FE:                     \n"
    // r2 hold EXC_RETURN value at this moment
    " push  {r2}                  \n"
    " bl    SVC_FE_handler        \n"
    //  design could be no return
    " pop   {r2}                  \n"
    // regular return;
    " bx    r2                    \n"
  );
  __asm volatile
  (
    // r2 hold EXC_RETURN value at this moment
    " push  {r2}                  \n"
    " bl    SVC_Handler_apphook   \n"
    " pop   {r2}                  \n"
    // regular return;
    " bx    r2                    \n"
  );
}
#endif

#include	"nxRTOSConfig.h"

 __attribute__ (( weak ))
 __attribute__ (( naked ))
 void SVC_00_handler()
 {
 }
 __attribute__ (( weak ))
 __attribute__ (( naked ))
void SVC_00_exit()
{
}

/* SVC_01_handler designed to start first Job into a Thread_context.
* Through general SVC_Handler entrance processing, at the start point of
* SVC_01_handler, it expected that the EXC_RETURN  value is in R2 and
* LR contains the PC address back to  SVC_Handler.
* R0 reg contains  Thread_SP, which point to "SP_after_entering_interrupt"
*
* " stmia   r0!,{r4-r7}   // save r4 to r7 into mem addressed by r0.
*/

__attribute__ (( weak ))
__attribute__ (( naked ))
void SVC_01_handler()
{
  __asm volatile
  (
    //  load the pointer  firstJCB into r0.
    " ldr r0, [r0]                      \n"
    // invoke generic Scheduler_firstTCB to.
    " bl  firstTCB                      \n"
    // /* now r0 have the pointer to current TCB */
    " ldr r0, [r0]                      \n"
    // get the new Thread_SP in r0
  );
  __asm volatile
  ( //  pop r4-r11 from thread_stack, total 8 registers
    //  r0 holds the stack_pointer of the thread_stack
    " ldmia r0!,  {r4-r7}               \n"
    " mov   r8,   r4                    \n"
    " mov   r9,   r5                    \n"
    " mov   r10,  r6                    \n"
    " mov   r11,  r7                    \n"
    " ldmia r0!,  {r4-r7}               \n"
  );
  __asm volatile
  ( //  pop lr from thread_stack
    " ldmia r0!,  {r2}                  \n"
    " mov   lr,   r2                    \n"
  );
  __asm volatile
  ( /* EXC_RETURN value =0xFFFFFFF9-Thread Stack uses MSP */
    " ldr r3,   =0xFFFFFFF9             \n"
    " cmp r2,   r3                      \n"
    " bne label_no_msp_adjust           \n"
    " msr msp,  r0                      \n"
    "label_no_msp_adjust: nop           \n"
  );
  __asm volatile
  ( /* EXC_RETURN value =0xFFFFFFFD-Thread Stack uses PSP */
    " ldr r3,   =0xFFFFFFFD             \n"
    " cmp r2,   r3                      \n"
    " bne label_no_psp_adjust           \n"
    " msr psp,  r0                      \n"
    "label_no_psp_adjust: nop           \n"
  );
  // it has to enable global irq before return from PenSV
  __enable_irq();
  __asm volatile
  ( // regular return;
    " bx  LR                            \n"
  );
  while(1);
  __asm volatile
  (
    " .align 4                          \n"
  );
}

__attribute__ (( weak ))
__attribute__ (( naked ))
void SVC_06_handler()
{
  __asm volatile
  ( // regular return to caller
    " bx    lr                \n"
    // r2 hold EXC_RETURN value from caller
    // return to thread
    " bx    r2                \n"
    " loop4ever:  nop         \n"
    " b  loop4ever            \n"
  );
}

// simply in and out stub
__attribute__ (( weak ))
__attribute__ (( naked ))
void SVC_07_handler()
{
  __asm volatile
  (
    // r2 hold EXC_RETURN value from caller
    // return to thread
    "	bx  r2              \n"
  );
  return;
}

__attribute__ (( weak ))
void SVC_08_handler()
{
	return;
}

// ISR hook for TestSysCriticalLevelFromISR
__attribute__ (( weak ))
void SVC_FE_handler()
{
  xTestSysCriticalLevelFromISR();
  return;
}

///////////////////////////////////////////////////////////////////////////////
/* SVC_FF_handler designed to terminate current  Thread_context.
  * Through general SVC_Handler entrance processing, at the start point of
  * SVC_FF_handler, it expected that the EXC_RETURN  value is in R2 and
  * LR contains the PC address back to  SVC_Handler.
  * R0 reg contains  Thread_SP, which point to "SP_after_entering_interrupt"
  *
  * "    stmia   r0!,{r4-r7}   // save r4 to r7 into mem addressed by r0.
  */
///////////////////////////////////////////////////////////////////////////////

__attribute__ (( weak ))
__attribute__ (( naked ))
void SVC_FF_handler()
{
  __asm volatile
  (
    " ldr r0, [r0]              \n" //  load the pointer  termiTCB into r0.
    // invoke generic Scheduler_nextTCB to abundant currentTCB
    " bl  nextTCB               \n"
    // /* now r0 have the pointer to current TCB */
    " ldr r0, [r0]              \n" // get the new Thread_SP in r0
  );
  __asm volatile
  ( //  pop r4-r11 from thread_stack, total 8 registers
    //  r0 holds the stack_pointer of the thread_stack
    " ldmia r0!,  {r4-r7}       \n"
    " mov   r8,   r4            \n"
    " mov   r9,   r5            \n"
    " mov   r10,  r6            \n"
    " mov   r11,  r7            \n"
    " ldmia r0!,  {r4-r7}       \n"
  );
  __asm volatile
  ( //  pop lr from thread_stack
    " ldmia r0!,  {r2}          \n"
    " mov   lr,   r2            \n"
  );
  __asm volatile
  ( /* EXC_RETURN value =0xFFFFFFF9-Thread Stack uses MSP */
    " ldr r3,   =0xFFFFFFF9           \n"
    " cmp r2,   r3                    \n"
    " bne label_no_msp_adjust_01      \n"
    " msr msp,  r0                    \n"
    "label_no_msp_adjust_01:  nop     \n"
  );
  __asm volatile
  ( /* EXC_RETURN value =0xFFFFFFFD-Thread Stack uses PSP */
    " ldr r3,   =0xFFFFFFFD                     \n"
    " cmp r2,   r3                              \n"
    " bne label_no_psp_adjust_01                \n"
    " msr psp,  r0                              \n"
    "label_no_psp_adjust_01: nop                \n"
  );
  // it has to enable global irq before return from PenSV
  __enable_irq();
  __asm volatile
  ( // regular return;
    " bx  LR                                  \n"
  );
  // should never go beyond
  while(1);
  __asm volatile
  (
      "   .align 4                    \n"
  );
}

/*  stub for SVC_Handler customer application hook */
__attribute__ (( weak ))
void SVC_Handler_apphook()
{
  return;
}
