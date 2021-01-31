/* arch4rtos_svc.h
 *
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 * *
 * 1 tab == 2 spaces!
 */

#ifndef  ARCH4RTOS_SVC_H        ///{{{
#define  ARCH4RTOS_SVC_H        ///{{{
#if   0
#include  "arch4rtos_svc.txt.h"
#endif

#include  "arch4rtos.h"

#define SVC(code)     asm volatile ("svc %0" : : "I" (code) )

extern
void  SVC_00_exit(void);
extern
uint32_t  xIsSVC_00_CurrentRunning(void);

#endif
/// }}}	ARCH4RTOS_SVC_H  }}}
