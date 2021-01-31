/* arch4rtos_prekernel_init.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  nxrtos or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 2 spaces!
 */
/*-----------------------------------------------------------------------------
 * Implementation of SysCriticalLevel for the architecture of
 * ARM CM4F.
 * Refer to arch4rtos.tx for design consideration.
 *---------------------------------------------------------------------------*/

#include  "arch4rtos_prekernel_init.h"
#include  "CMSIS_Device.h"
#include  "nxRTOSConfig.h"

uint32_t arch4rtos_prekernel_init()
{
  ///     {{{
  //NVIC_SetPriority(SVCall_IRQn, (1 << __NVIC_PRIO_BITS) -1);
  // SetPriority SVCall_IRQn to RTOS_SYSCRITICALLEVEL safe but may over kill
  // SetPriority to lowest and then raise the critical_level inside when needed
  __NVIC_SetPriority(SVCall_IRQn, RTOS_SVC_ENTRY_PRIORITY);
  // SysTick should set to lowest ISRCRITICALLEVEL,
  // but can be higher up to RTOS_SYSCRITICALLEVEL
  __NVIC_SetPriority(SysTick_IRQn, LOWEST_ISRCRITICALLEVEL);
  // PendSV has to be lowest ISRCRITICALLEVEL
  __NVIC_SetPriority(PendSV_IRQn, LOWEST_ISRCRITICALLEVEL);
  ///     }}}
  return 0;
}
