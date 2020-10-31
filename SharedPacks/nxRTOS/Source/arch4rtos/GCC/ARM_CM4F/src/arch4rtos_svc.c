/* arch4rtos_svc.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */
/*-----------------------------------------------------------------------------
 * Implementation of functions defined in arch4rtos.h for the architecture of
 * ARM CM4F.
 * The file may move to shared_arch4rtos/ or ARM_CMX/ with if defined
 * (__ARM_ARCH_XX__)  && (__ARM_ARCH_XX__ == ???) to treat different arch
 *
 * Refer to arch4rtos.tx for design consideration.
 *---------------------------------------------------------------------------*/

#include "arch4rtos.h"
#include "arch4rtos_criticallevel.h"
#include "nxRTOSConfig.h"

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
#if (defined (__ARM_ARCH_7EM__) && (__ARM_ARCH_7EM__ == 1))
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
    // switch(svc_number/*is an integer in the range 0-255.*/)
    " ldrb  r3,   [r3, #0]        \n"
    // case 00:
    " cmp   r3,   #0              \n"
    " beq   svc_00                \n"
    " cmp   r3,   #1              \n"
    " beq   svc_01                \n"
    " cmp   r3,   #6              \n"
    " beq   svc_06                \n"
    " cmp   r3,   #7              \n"
    " beq   svc_07                \n"
    " cmp   r3,   #8              \n"
    " beq   svc_08                \n"
    " cmp   r3,   #254            \n"
    " beq   svc_FE                \n"
    " cmp   r3,   #255            \n"
    " beq   svc_FF                \n"
    // default:
    " svc_number_default:         \n"
    // svc_xx below
    " svc_00:                     \n"
    " bx    lr                    \n"
    " svc_01:                     \n"
    // r2 hold EXC_RETURN value at this moment
    " bl    SVC_01_handler        \n"
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

    " svc_FE:                     \n"
    // r2 hold EXC_RETURN value at this moment
    " push  {r2}                  \n"
    " bl    SVC_FE_handler        \n"
    //  design could be no return
    " pop   {r2}                  \n"
    // regular return;
    " bx    r2                    \n"

    " svc_FF:                     \n"
    // r2 hold EXC_RETURN value at this moment
    " bl      SVC_FF_handler      \n"
    //  no return
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
    " ldr   r0,   [r0]                       \n" //  load the pointer  firstJCB into r0.
    // invoke generic Scheduler_firstTCB to.
    " bl    firstTCB                         \n"
    // /* now r0 have the pointer to current TCB */
    " ldr   r0,   [r0]                       \n" // get the new Thread_SP in r0
  );
  // after this point will be same as after pickTCB in PendSV_Handler
  __asm volatile
  ( //  pop r4-r11 from thread_stack, total 8 registers
    //  r0 holds the stack_pointer of the thread_stack
    " ldmia r0!,  {r4-r11}                    \n"
  );

  __asm volatile
  ( //  pop  lr from thread_stack
    " ldmia r0!,  {lr}                        \n"
  );
  __asm volatile
  ( // assume Thread_context using PSP, but more work to check it up after.
    // preset PSP
    " msr   psp,  r0                          \n"
    //  Write value of 0x4 to R3, flags get up
    " movs  r3,   #4                          \n"
    //The TST instruction performs a bitwise AND operation
    //  on the value in R2 and the value in R3
    " TST   lr,   r3                          \n"
    // NE means EXC_RETURN value =0xFFFFFFFD, aka. Thread SP uses PSP
    " bne   label_no_msp_adjust_01            \n"
    /* EXC_RETURN value =0xFFFFFFF9-Thread Stack uses MSP */
    " ldr   r3,   =0xFFFFFFF9                 \n"
    " cmp   lr,   r3                          \n"
    " bne   label_no_msp_adjust_01            \n"
    " msr   msp,  r0                          \n"
    " movs  r3,   #8                          \n"
    " TST   lr,   r3                          \n"
    // NE means EXC_RETURN value =0xFFFFFFF1, aka. return to ISR
    // panic.
    " BEQ   label_panic_loop                  \n"
    " label_no_msp_adjust_01: nop             \n"
  );
  __set_BASEPRI(0);
  // it has to enable global irq before return from PenSV
  __enable_irq();
  __asm volatile
  ( // regular return;
    " bx  LR                          \n"
  );
  __asm volatile
  ( // regular return;
    " label_panic_loop:               \n"
    " b   label_panic_loop            \n"
  );
}
__attribute__ (( weak ))
__attribute__ (( naked ))
void SVC_06_handler()
{
  __asm volatile
  ( // regular return to caller
    " bx    lr                    \n"
    // r2 hold EXC_RETURN value from caller
    // return to thread
    " bx    r2                    \n"
  );
  __asm volatile
  (
    " loop4ever:                  \n"
  );
  while(1);
}
__attribute__ (( weak ))
__attribute__ (( naked ))
void SVC_07_handler()
{
  __asm volatile
  (
    // r2 hold EXC_RETURN value from caller
    // return to thread
    " bx  r2                      \n"
  );
  return;
}

__attribute__ (( weak ))
void SVC_08_handler()
{
  return;
}

__attribute__ (( weak ))
void SVC_FE_handler()
{
  xTestSysCriticalLevelFromISR();
  return;
}
/* SVC_FF_handler designed to terminate current  Thread_context.
  * Through general SVC_Handler entrance processing, at the start point of
  * SVC_FF_handler, it expected that the EXC_RETURN  value is in R2 and
  * LR contains the PC address back to  SVC_Handler.
  * R0 reg contains  Thread_SP, which point to "SP_after_entering_interrupt"
  *
  * "    stmia   r0!,{r4-r7}   // save r4 to r7 into mem addressed by r0.
  */
__attribute__ (( weak ))
__attribute__ (( naked ))
void SVC_FF_handler()
{
  __asm volatile
  ( //  load the pointer  termiJCB into r0.
    " ldr   r0,   [r0]                          \n"
  );
  __asm volatile
  ( // invoke generic Scheduler_nextTCB to abundant currentTCB
    " bl    nextTCB                             \n"
  );
  __asm volatile
  ( // /* now r0 have the pointer to current TCB */
    " ldr   r0,   [r0]                          \n"
    // get the new Thread_SP in r0
  );
  __asm volatile
  ( //  pop r4-r11 from thread_stack, total 8 registers
    //  r0 holds the stack_pointer of the thread_stack
    " ldmia r0!,  {r4-r11}                      \n"
  );
  __asm volatile
  ( //  pop  lr from thread_stack
    " ldmia r0!,  {lr}                          \n"
  );
  __asm volatile
  ( // assume Thread_context using PSP, but more work to check it up after.
    // preset PSP, it is OK to change PSP if the Thread_context using MSP
    " msr   psp,  r0                    \n"
    //  Write value of 0x4 to R3, flags get up
    " movs  r3,   #4                    \n"
    //The TST instruction performs a bitwise AND operation
    //  on the value in R2 and the value in R3
    " TST   lr,   r3                    \n"
    // NE means EXC_RETURN value =0xFFFFFFFD, aka. Thread SP uses PSP
    " bne   label_no_msp_adjust_FF      \n"
    /* EXC_RETURN value =0xFFFFFFF9-Thread Stack uses MSP */
    " ldr   r3,   =0xFFFFFFF9           \n"
    " cmp   lr,   r3                    \n"
    " bne   label_no_msp_adjust_FF      \n"
    " msr   msp,  r0                    \n"
    " movs  r3,   #8                    \n"
    " TST   lr,   r3                    \n"
    // NE means EXC_RETURN value =0xFFFFFFF1, aka. return to ISR
    // panic.
    " BEQ   label_panic_loop            \n"
    " label_no_msp_adjust_FF: nop       \n"
  );
  // clear BASEPRI
  __set_BASEPRI(0);
  // it has to enable global irq before return from PenSV
  __enable_irq();
  __asm volatile
  ( // regular return;
    " bx  LR                            \n"
  );
}

__attribute__ (( weak ))
void SVC_Handler_apphook()
{
  return;
}
