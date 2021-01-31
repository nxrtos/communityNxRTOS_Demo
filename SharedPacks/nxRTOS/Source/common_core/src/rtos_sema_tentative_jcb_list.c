/**
  ******************************************************************************
  * @file           : rtos_sema_tentative_jcb_list.c
  * @brief          :
  * ****************************************************************************
  * @attention
  *                 nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * 1 tab == 2 spaces!
 */

/*-----------------------------------------------------------------------------

 *---------------------------------------------------------------------------*/

#include  "rtos_sema_tentative_jcb_list.h"
#include  "rtos_semaphore.h"

#if  (SUPPORT_SEMA_ATTR_JOB_REASSIGN) //  {{{
// community version does NOT support SEMA_CONDITION_JOB
#endif  //  #if  (SUPPORT_SEMA_ATTR_JOB_REASSIGN) }}}
