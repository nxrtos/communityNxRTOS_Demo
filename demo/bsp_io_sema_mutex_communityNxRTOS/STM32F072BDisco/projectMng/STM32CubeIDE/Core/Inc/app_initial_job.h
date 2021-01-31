/**
  ******************************************************************************
  * @file           : initial_job.h
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

#ifndef  _APP_INITIAL_JOB_H
#define  _APP_INITIAL_JOB_H



/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp.h"
#include "arch4rtos.h"
#include "rtos_jcb_ready_list.h"
#include "rtos_start_kernel.h"
#include "rtos_tick_process.h"
#include "arch4rtos_criticallevel.h"
#include "thread_priority.h"
#include "rtos_semaphore.h"
#include "rtos_mutex.h"
#include "rtos_commit_job.h"
#include "job_led_blink.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/**
  * @brief  The application entry point.
  * @retval int
  */
extern int app_initial_Job_Handler(void * par);

#endif
/************************ (C) COPYRIGHT  *****END OF FILE****/
