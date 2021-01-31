/**
  ******************************************************************************
  * @file           : arch4rtos_svc.c
  * @brief          :
  ******************************************************************************
  * nxRTOS Kernel V0.0.1
  * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
  *
  * 1 tab == 2 spaces!
  */
/*-----------------------------------------------------------------------------
 * Implementation of SVC_Handler  for ARM_CM0.
 * The file may move to shared_arch4rtos/ or ARM_CMX/ with if defined
 * (__ARM_ARCH_XX__)  && (__ARM_ARCH_XX__ == 1) to treat different arch
 *
 * Refer to arch4rtos_svc.txt.h for design consideration.
 *---------------------------------------------------------------------------*/

#include "arch4rtos_svc.h"
#include "arch4rtos_criticallevel.h"
#include "arch4rtos_testcriticallevel.h"
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
  __asm volatile(".global SVC_Handler_Main\n");

  // {{{ Cortex-M0 means (__CORTEX_M == 0), and so (__ARM_ARCH_6M__ == 1) {{{
#if (defined (__ARM_ARCH_6M__) && (__ARM_ARCH_6M__ == 1))
  /// TODO:: update SVCHandler_main to not rely on EXC_RETURN in R2.
  //  keep it in LR
  __asm volatile
  (
    " MOVS  R0, #4                      \n"
    " MOV   R2, LR                      \n"
    " TST   R0, R2                      \n"
    " BEQ   invoker_stacking_used_MSP   \n"
    " MRS   R0, PSP                     \n"
    //" b     SVCHandler_main             \n"
    // alt way to SVC_Handler_main
    " LDR   R1, =SVCHandler_main        \n"
    " BX    R1                          \n"

    " invoker_stacking_used_MSP:        \n"
    " MRS   R0, MSP                     \n"
    //" b     SVCHandler_main             \n"
    // alt way to SVC_Handler_main
    " LDR   R1, =SVCHandler_main        \n"
    " BX    R1                          \n"

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

  /// " b SVCHandler_main"
  __asm volatile
  ( //
    " b SVCHandler_main         \n"
  );
}


/**
  * @brief  SVC dispatching function.
  * @param  pointer to ISR_enter_frame. svc_args[6] holds the value of PC when
  *         return from SVC_Handler ISR.
  * @retval None
  */

__attribute__ (( naked ))
void SVCHandler_main(void * sp)
{
  __asm volatile
  (
    // svc_number = ((char *)svc_args[6])[-2];
    " ldr   r3,   [r0, #24]       \n"
    " sub   r3,   #2              \n"
    // switch(svc_number)
    " ldrb  r3,   [r3, #0]        \n"
    // case ff
    " cmp   r3,   #0xff           \n"
    //" beq   svc_ff                \n"
    // alt to
    " bne   skip_FF               \n"
    " b     SVC_FF_handler        \n"
    " skip_FF:                    \n"
    // case 00:
    " cmp   r3,   #0              \n"
    " bne   skip_00               \n"
    " b     SVC_00_handler        \n"
    " skip_00:                    \n"
    // case 01:
    " cmp   r3,   #1              \n"
    " bne   skip_01               \n"
    " b     SVC_01_handler        \n"
    " skip_01:                    \n"
    // case 06:
    " cmp   r3,   #6              \n"
    " bne   skip_06                \n"
    " b     SVC_06_handler         \n"
    " skip_06:                    \n"
    // case 07:
    " cmp   r3,   #7              \n"
    " bne   skip_07               \n"
    " b     SVC_07_handler        \n"
    " skip_07:                    \n"
    // case 08:
    " cmp   r3,   #8              \n"
    " bne   skip_08               \n"
    " b     SVC_08_handler        \n"
    " skip_08:                    \n"
    // case 0xFE:
    " cmp   r3,   #254            \n"
    " bne   skip_FE               \n"
    " b     SVC_FE_handler        \n"
    " skip_FE:                    \n"
    // default:
    " b     loop4ever             \n"
    // regular return;
    " bx    lr                    \n"
  );
  __ISB();
  __DSB();
  __asm volatile
  (
  // regular return;
  " bx    lr                    \n"
  );
}

#include  "nxRTOSConfig.h"

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


 /**
  * @brief  SVC_01_handler designed to start first Job into a Thread_context.
  *         It abandons the original ISR_Entry_Frame and fabric a new through
  *         firstTCB => arch4rtos_pInitThreadStack
  * @param  theJCB: pointer to the Init_JCB
  * @retval None
  *
  * " stmia   r0!,{r4-r7}   // save r4 to r7 into mem addressed by r0.
  */

__attribute__ (( naked ))
void SVC_01_handler(void * theJCB)
{
  __disable_irq();  // disable isr may not really needed, but just for secure
  __asm volatile
  (
   //  load the pointer  firstJCB into r0.
   " ldr r0, [r0]                      \n"
   // invoke generic Scheduler_firstTCB to.
   " bl  firstTCB                      \n"
   // after this point it should be same as PendSV last part
   // /* now r0 have the pointer to current TCB */
   " ldr r0, [r0]                      \n"
   // get the new Thread_SP in r0
  );
  //  pop r4-r11 from thread_stack, total 8 registers
  //  r0 holds the stack_pointer of the thread_stack
  // {{{ Cortex-M0 means (__CORTEX_M == 0), and so (__ARM_ARCH_6M__ == 1) {{{
#if (defined (__ARM_ARCH_6M__) && (__ARM_ARCH_6M__ == 1))
  __asm volatile
  (
   " ldmia r0!,  {r4-r7}               \n"
   " mov   r8,   r4                    \n"
   " mov   r9,   r5                    \n"
   " mov   r10,  r6                    \n"
   " mov   r11,  r7                    \n"
   " ldmia r0!,  {r4-r7}               \n"
  );
#else
  // }}} Cortex-M0 means (__CORTEX_M == 0), and so (__ARM_ARCH_6M__ == 1) }}}

  // {{{ Cortex-M4 means (__CORTEX_M == 4), and so (__ARM_ARCH_7EM__ == 1) {{{
#if (defined (__ARM_ARCH_7EM__     ) && (__ARM_ARCH_7EM__     == 1))
  __asm volatile
  (
    " ldmia r0!,  {r4-r11}                    \n"
  );
#endif
  // }}} Cortex-M4 means (__CORTEX_M == 4), and so (__ARM_ARCH_7EM__ == 1) }}}
#endif

  __asm volatile
  ( //  pop lr from thread_stack
   " ldmia r0!,  {r2}                  \n"
   " mov   lr,   r2                    \n"
  );

  __asm volatile  // decide in arch4rtos_pInitThreadStack
  ( /* EXC_RETURN value =0xFFFFFFFD- exit to Thread mode Stack uses PSP */
    // this is designated for nxRTOS Thread context for Cortex-MX
   " ldr r3,   =0xFFFFFFFD              \n"
   " cmp r2,   r3                       \n"
   " bne exit_skip_thread_psp           \n"
   " msr psp,  r0                       \n"
  );
  __ISB();
  __DSB();
  __enable_irq();
  __asm volatile
  (
   " bx  LR                             \n"
   " exit_skip_thread_psp:              \n"
  );

  // as current design, it shouldn't reach this point but
  // just for potential extension
  __asm volatile
  ( /* EXC_RETURN value =0xFFFFFFF9-exit to Thread mode Stack uses MSP */
   " ldr r3,   =0xFFFFFFF9              \n"
   " cmp r2,   r3                       \n"
   " bne exit_skip_thread_msp           \n"
   " msr msp,  r0                       \n"
  );
  __ISB();
  __DSB();
  __enable_irq();
  __asm volatile
  (
   " bx  LR                             \n"
   " exit_skip_thread_msp:              \n"
  );

  // as current design, it shouldn't reach this point but
  // just for potential extension
  __asm volatile
  ( /* EXC_RETURN value =0xFFFFFFF1, exit to Handler mode Stack must uses MSP */
   " ldr r3,   =0xFFFFFFF1              \n"
   " cmp r2,   r3                       \n"
   " bne exit_skip_isr_msp              \n"
   " msr msp,  r0                       \n"
  );
  __ISB();
  __DSB();
  __enable_irq();
  __asm volatile
  (
   " bx  LR                             \n"
   " exit_skip_isr_msp:                 \n"
  );

  // catch error
  while(1);

  __asm volatile
  ( /* general exit method from SVC */
   " exit_svc:                          \n"
  );
  __ISB();
  __DSB();
  __enable_irq();
  __asm volatile
  (
   " bx  LR                             \n"
  );
}

__attribute__ (( weak ))
__attribute__ (( naked ))
void SVC_06_handler()
{
  __asm volatile
  ( // regular return to caller
    " bx    lr                \n"
    " loop4ever:  nop         \n"
    " b     loop4ever         \n"
  );
}

// simply in and out stub
__attribute__ (( weak ))
__attribute__ (( naked ))
void SVC_07_handler()
{
  __asm volatile
  (
    " bx  lr            \n"
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
/* SVC_FF_handler designed to terminate current Thread.
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
    // invoke generic Scheduler_pickTCB_PostTerm to abundant currentTCB
    " bl  pickTCB_PostTerm               \n"
    // /* now r0 have the pointer to current TCB */
    " ldr r0, [r0]              \n" // get the new Thread_SP in r0
  );

  // after this point will be same to last part of SVC_01 and PendSV
  // {{{ Cortex-M0 means (__CORTEX_M == 0), and so (__ARM_ARCH_6M__ == 1) {{{
#if (defined (__ARM_ARCH_6M__) && (__ARM_ARCH_6M__ == 1))
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
  __asm volatile  // this is designated case
  ( /* EXC_RETURN value =0xFFFFFFFD- exit to Thread, Stack uses PSP */
    " MOVS  R3, #4                  \n"
    " MOV   R2, LR                  \n"
    " TST   R3, R2                  \n"
    " BEQ   skip_psp_adjust         \n"
    //  EXC_RETURN value =0xFFFFFFFD- exit to Thread, Stack uses PSP
    " msr   psp,  r0                \n"
  );
  __ISB();
  __DSB();
  __asm volatile
  ( //
    " b   exit_SVC                  \n"
  );

  __asm volatile
  ( // not for currrent design, just in case for extension
    " skip_psp_adjust:              \n"
    /* EXC_RETURN value =0xFFFFFFF9 -exit to Thread mode Stack uses MSP */
    // EXC_RETURN value =0xFFFFFFF1 -exit to Handler mode Stack must using MSP.
    " msr msp,  r0                  \n"
  );
  __ISB();
  __DSB();
  // catch error , at current implementation, LR has to be 0xfffffffd
  __asm volatile(
    " ldr r3,   =0xFFFFFFFD              \n"
    " cmp lr,   r3                       \n"

      "BNE   debug_force      \n"
      "through:     \n"
  ) ;

  __asm volatile
  ( //
    " b   exit_SVC                    \n"
  );
#else
  // }}} Cortex-M0 means (__CORTEX_M == 0), and so (__ARM_ARCH_6M__ == 1) }}}

  // {{{ Cortex-M4 means (__CORTEX_M == 4), and so (__ARM_ARCH_7EM__ == 1) {{{
#if (defined (__ARM_ARCH_7EM__) && (__ARM_ARCH_7EM__ == 1))
  __asm volatile
  ( //  pop r4-r11 from thread_stack, total 8 registers
    //  r0 holds the stack_pointer of the thread_stack
    " ldmia r0!,  {r4-r11}          \n"
  );
  __asm volatile
  ( //  pop  lr from thread_stack
    " ldmia r0!,  {lr}              \n"
  );
  __asm volatile(
    "TST lr, #4     \n"
    "ITE EQ         \n"
    "MSREQ MSP, r0  \n"
    "MSRNE PSP, r0  \n"
  ) ;

  // catch error , at current implementation, LR has to be 0xfffffffd
  __asm volatile(
    " ldr r3,   =0xFFFFFFFD              \n"
    " cmp lr,   r3                       \n"

      "BNE   debug_force      \n"
      "through:     \n"
  ) ;
  //"BNE   forever_loop      \n"

  // clear BASEPRI
  __set_BASEPRI(0);
#endif
  // }}} Cortex-M4 means (__CORTEX_M == 4), and so (__ARM_ARCH_7EM__ == 1) }}}
#endif
  __asm volatile
  ( //
    " exit_SVC:                     \n"
  );
  // it has to enable global irq before return from PenSV
  __enable_irq();
  __asm volatile
  ( // regular return;
    " bx  LR                        \n"
  );
  // should never go beyond
  __asm volatile
  ( //
    " debug_force:  nop \n"
    " MOV   LR , R3     \n"
    " b     through     \n"
  ) ;

  __asm volatile
  ( // regular return;
    " forever_loop: \n"
  ) ;
  while(1);
}

__attribute__ (( weak ))
void SVC_Handler_apphook()
{
  return;
}
