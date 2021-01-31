/**
  ******************************************************************************
  * @file           : mutex_demo_job.h
  * @brief          :
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */

#ifndef __MUTEX_DEMO_JOB_H  //  {{{
#define __MUTEX_DEMO_JOB_H  //  {{{
#if  0
#include  "mutex_demo_job.txt.h"
#endif
#include "nxRTOSConfig.h"

#define  MUTEXDEMO_JOB_PRIORITY         RTOS_NORMAPP_THREAD_PRIORITY
#define  MUTEXDEMO_STACK_SIZE           0x400 /* THREAD_STACKSIZE */

#ifdef __cplusplus
 extern "C" {
#endif
extern  int start_mutex_demo(void * par);
#ifdef __cplusplus
}
#endif

#endif  //  }}} __MUTEX_DEMO_JOB_H  }}}
