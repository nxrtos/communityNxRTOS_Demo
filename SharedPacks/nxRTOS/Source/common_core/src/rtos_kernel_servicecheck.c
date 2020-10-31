/*  rtos_kernel_servicecheck.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */
#include  "rtos_kernel_servicecheck.h"
#include  "arch4rtos_criticallevel.h"
#include  "nxRTOSConfig.h"
#include  "list_tcb.h"
#include  "list_jcb.h"

int  rtos_kernel_scheduler_check()
{
    int ret;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // critical section enter   {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);
    if(((pxCurrentTCB == NULL) &&  (pxReadyListJCB != NULL))
       || // more check on pxReadyListTCB
       ((pxCurrentTCB != NULL) &&
        (pxReadyListJCB != NULL) &&
        (pxReadyListJCB->uxPriority < pxCurrentTCB->uxPriority)
       )
      )
    {
      arch4rtosReqSchedulerService();
      ret = 1;
    }
    else
    {   /* TODO do we want to clear the request_to_service up in the case a Job
         * or Thread been removed from readList ? The case of a readyJob been
         * canceled or waked up Thread been revoked resource grant from Mute or
         * Semaphore. */
        ret = 0;
    }
    // critical section exit    }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
    return  ret;
}
