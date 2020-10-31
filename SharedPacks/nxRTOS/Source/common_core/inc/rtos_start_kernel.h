/* rtos_start_kernel.h
 *
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/

#ifndef RTOS_START_KERNEL_H  // {{{
#define RTOS_START_KERNEL_H  // {{{
#if  0
#include  "rtos_start_kernel.txt.h"
#endif

int  rtos_start_kernel(void);

// app should provide customized version
int initial_Job_Handler(void * par);
// app should provide customized version
int idle_Job_Handler(void * par);

#endif	// }}} START_KERNEL_H }}}
