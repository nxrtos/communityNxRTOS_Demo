/* arch4rtos_firstjob.c
 *
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved
 *
 * 1 tab == 4 spaces!
 */


/*-----------------------------------------------------------------------------
 * Implementation of functions defined in arch4rtos.h for the architecture of
 * ARM CM4F.
 * Refer to arch4rtos.tx for design consideration.
 *---------------------------------------------------------------------------*/
#include "arch4rtos.h"
#include "arch4rtos_firstjob.h"
#include "rtos_stackspace.h"

static void * _getPoint(void * p);

// refer to void vPortStartFirstTask( void )
// refer to         prvPortStartFirstTask( void )  for ARM_CM4F for the way
// to get reset_SP
__attribute__ (( weak ))
__attribute__ (( naked ))
int  arch4rtos_firstjob(JCB_t *     firstJCB)
{

    // StackType_t * the_sp = arch4rtos_initial_sp_for_run2term();
    // StackType_t * current_sp = (StackType_t *)  __get_MSP();
    uint32_t  current_sp;
    //	 uint32_t  reset_sp = *((uint32_t  *)0);

    // sanity check:: this function has to be invoked from Thread_context.
    while(__get_IPSR());

    NVIC_SetPriority(SVCall_IRQn, HIGHEST_SYSCRITICALLEVEL);
    //NVIC_SetPriority(SVCall_IRQn, LOWEST_ISRCRITICALLEVEL);
    //NVIC_SetPriority(SVCall_IRQn, (0x01 << __NVIC_PRIO_BITS) -1);
    {
        uint32_t pri = NVIC_GetPriority(SVCall_IRQn);
        NVIC_SetPriority(SVCall_IRQn, pri);
        __DSB();
        __ISB();
    }

    if(!(CONTROL_SPSEL_Msk &  __get_CONTROL()))
    {  // if current Thread_context use MSP, switch to PSP.
        current_sp = __get_MSP();
        __set_PRIMASK(1);
        __set_PSP(current_sp);
        __set_CONTROL(CONTROL_SPSEL_Msk); // [1]: 1 = PSP is the current stack pointer
        __DSB();
        __ISB();
        __set_MSP((uint32_t)INITIAL_ISR_StackPoint);
        setDetection_ISR_Stack_Overflow();
    }

    __set_PRIMASK(0);
    // exit from first job, "svc  X" to handle
    __asm volatile
    (   //  SVC in and out test
        //"   svc         0 \n"
        "                   \n"
    );
    __DSB();
    __ISB();

    // make sure firstJCB in R0
    _getPoint(firstJCB);
    __asm volatile
    (
        //  assume Thread mode with PSP, in case of MSP fix it later.
        "   svc             01      \n"         // SVC_01_handler()
    );

    //  assume Thread mode with PSP, in case of MSP fix it later.
    while(1)
    {
        __DSB();
        __ISB();
    }

    return 0;
}

static void * _getPoint(void * p)
{
    return p;
}

