/* rtos_create_tcb.h
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  nxrtos or its affiliates.  All Rights Reserved.
 *
 *  1 tab == 2 spaces!
 */

/*-----------------------------------------------------------------------------
 * create a new TCB from a readyJCB
 *---------------------------------------------------------------------------*/
#ifndef RTOS_CREATE_TCB_H   /// {{{
#define RTOS_CREATE_TCB_H   /// {{{

#if   0
#include  "rtos_create_tcb.txt.h"
#endif

#include  "rtos_tcb_live.h"
#include  "rtos_jcb.h"

#ifdef __cplusplus
extern "C" {
#endif

extern  LiveTCB_t * rtos_create_tcb(JCB_t * readyJCB);

#ifdef __cplusplus
}
#endif

#endif      // }}} RTOS_CREATE_TCB_H  }}}
