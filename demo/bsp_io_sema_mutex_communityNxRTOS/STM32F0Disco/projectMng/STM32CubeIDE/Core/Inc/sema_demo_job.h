/**
  ******************************************************************************
  * @file           : sema_job_led_blink.h
  * @brief          :
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */

#ifndef __SEMA_DEMO_JOB_H  //  {{{
#define __SEMA_DEMO_JOB_H  //  {{{

#if  0
#include  "sema_demo_job.txt.h"
#endif
#include "nxRTOSConfig.h"

#define  SEMADEMO_JOB_PRIORITY         RTOS_NORMAPP_THREAD_PRIORITY
#define  SEMADEMO_STACK_SIZE           0x400 /* THREAD_STACKSIZE */

#ifdef __cplusplus
 extern "C" {
#endif
extern  int start_sema_demo(void * par);
#ifdef __cplusplus
}
#endif

#endif  //  }}} __SEMA_DEMO_JOB_H  }}}
