/**
 ******************************************************************************
 * @file      arch4rtos_check_tcb.h
 * @author
 * @brief     check tcb and the contains in stack
 *
 ******************************************************************************
 * @attention
 */

#ifndef ARCH4RTOS_CHECK_TCB_H
#define ARCH4RTOS_CHECK_TCB_H
#if  0
#include  "arch4rtos_check_tcb.txt.h"
#endif
#include  "arch_basetype.h"
#include  "CMSIS_Device.h"
#include  "rtos_tcb_live.h"

#ifdef __cplusplus
extern "C" {
#endif

///{{{

void  arch4rtos_check_TCB(LiveTCB_t * theTCB);

///}}}
#ifdef __cplusplus
}
#endif

#endif /* ARCH4RTOS_CHECK_TCB_H */

