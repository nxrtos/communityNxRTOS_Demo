/**
  ******************************************************************************
  * @file           : job_button_detection.h
  * @brief          :
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
#ifndef     _JOB_BUTTON_DETECTION_H
#define     _JOB_BUTTON_DETECTION_H
#include    "stdint.h"
#include    "CMSIS_Device.h"
#include    "rtos_commit_job.h"

#ifdef __cplusplus
extern "C" {
#endif

extern  int button_detect_init_job_handler(void * par);
extern  int button_detection_job_handler(void * par);

#define  BUTTON_DETECT_JOB_NORM_PRIORITY	RTOS_HIGHEST_THREAD_PRIORITY
#define  BUTTON_DETECT_JOB_NORM_STACK_SIZE	0x400
#ifdef __cplusplus
}
#endif

#endif  //_JOB_BUTTON_DETECTION_H
