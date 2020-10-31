/**
  ******************************************************************************
  * @file           : app_initial_job.c
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include  "app_initial_job.h"
#include  "arch4rtos_testcriticallevel.h"
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

int app_initial_Job_Handler(void * par)
{
    //Sem_t     * theSem;
	JCB_t * initialJCB =(JCB_t *)par;

	// sanity check for initial_Job Thread
	if(initialJCB->threadEntryFunc != initial_Job_Handler)
	{
		while(1);
	}

	if(initialJCB->uxPriority != IrqPriorityMaskLevelNOMASK)
	{
		while(1);
	}
  
	xTestSysCriticalLevelFromThread();
	// TODO investigate
	xTestThreadPriority();

	BSP_LED_Init(LED_ALL);
	// init_job_handler drop ThreadPriority to lowest THREAD_PRIORITY
	// turned to be idle_job_handler
	xDropLiveThreadPriority(NULL, RTOS_LOWEST_THREAD_PRIORITY);

#if 0
    rtos_commit_deferjob(NULL,
                                                0 ,//   int iJobPar,
                                                 NULL,  ThreadStackTempStacking, 0x400 /*iStackSize*/,
                                                 test_softtimer_starter , /*pJobHandler*/
                                                 RTOS_HIGHEST_THREAD_PRIORITY +1 /*xJobPriority*/,
                                                 RelocateToThreadStack,  0);
#endif

#if 0
    rtos_commit_deferjob(NULL,
                                                0 ,//   int iJobPar,
                                                 NULL,  ThreadStackTempStacking, 0x400 /*iStackSize*/,
                                                 run2blck_test_starter , /*pJobHandler*/
                                                 RTOS_HIGHEST_THREAD_PRIORITY +1 /*xJobPriority*/,
                                                 RelocateToThreadStack,  0);
#endif

#if 0
    rtos_commit_deferjob(NULL,
                                                0 ,//   int iJobPar,
                                                 NULL,  ThreadStackTempStacking, 0x400 /*iStackSize*/,
                                                 run2blck_test_mutex_starter , /*pJobHandler*/
                                                 RTOS_HIGHEST_THREAD_PRIORITY +1 /*xJobPriority*/,
                                                 RelocateToThreadStack,  0);
#endif

#if     0
    rtos_commit_deferjob(NULL,
                                                0 ,//   int iJobPar,
                                                 NULL,  ThreadStackLongKeeping, 0x400 /*iStackSize*/,
                                                 mutex_jobcommit_test_mutex_starter , /*pJobHandler*/
                                                 RTOS_HIGHEST_THREAD_PRIORITY +1 /*xJobPriority*/,
                                                 RelocateToThreadStack,  0);
#endif

#if     0
    rtos_commit_deferjob(NULL,
                                                0 ,//   int iJobPar,
                                                 NULL,  ThreadStackLongKeeping, 0x400 /*iStackSize*/,
                                                 mempool_test_starter , /*pJobHandler*/
                                                 RTOS_HIGHEST_THREAD_PRIORITY +1 /*xJobPriority*/,
                                                 RelocateToThreadStack,  0);
#endif

#if     0
	// submit a deferred job with shared_Stack
	rtos_commit_job(NULL, 0, NULL,  ThreadStackTempStacking, 0 /*iStackSize*/,
	                                                blink_job_handler /*pJobHandler*/,
	                                                RTOS_HIGHEST_THREAD_PRIORITY /*xJobPriority*/,
	                                                NULL /* pConditionToRun */,
	                                                500 /*deferTicks*/);
#endif

#if     0
    rtos_commit_deferjob(NULL,
                                                greenLED  + (0x4 <<  theLED_NumBits ),//     bdrLED,
                                                 NULL,  ThreadStackLongKeeping, 0x400 /*iStackSize*/,
                                                 run2blck_blink_job_handler /*pJobHandler*/,
                                                 RTOS_HIGHEST_THREAD_PRIORITY +1 /*xJobPriority*/,
                                                 RelocateToThreadStack,  12);
#endif

#if     0
    rtos_commit_deferjob(NULL,
                                                0 ,
                                                NULL,  ThreadStackLongKeeping, 0x400 /*iStackSize*/,
                                                test_sem_blink_starter /*pJobHandler*/,
                                                 RTOS_HIGHEST_THREAD_PRIORITY +1 /*xJobPriority*/,
                                                 RelocateToThreadStack,  0);
#endif

#if 0
    rtos_commit_deferjob(NULL,
                                                theLED3,//orangeLED    bdrLED,

                                                theSem,  ThreadStackLongKeeping, 0x400 /*iStackSize*/,
                                                 run2blck_semblink_job_handler /*pJobHandler*/,
                                                 RTOS_HIGHEST_THREAD_PRIORITY +1 /*xJobPriority*/,
                                                 RelocateToThreadStack,  0);
#endif

    do
	{
#if     0
		{
		  static uint32_t	cnt_run = 0;

		  if (++cnt_run > 800)
		  {
			  cnt_run = 0;
			  // trigger NMI, check NMI_Handler(void) for LD3 toggle
			  SCB->ICSR |= SCB_ICSR_NMIPENDSET_Msk;
		  }
		}
#endif

#if     0
		{
		  static uint32_t	cnt_run = 0;

		  if (++cnt_run > 400)
		  {
			  cnt_run = 0;
			  // trigger PendSV, assume the PENDSV_Handler will be
			  // tolerate to the situation switch find fall back to the currentTCB
			  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
		  }
		}
#endif

	#if	0
	  {
		  static uint32_t	cnt_run = 0;

		  if (++cnt_run > 32000000)
		  {
			  cnt_run = 0;
	#if		01
			  uint32_t  priorityLevel_SysIrq;
	#if 01
			  priorityLevel_SysIrq = xGetSysIrqPriorityLevel(SysIRQ_PendSV);
			  priorityLevel_SysIrq += IrqPriorityMaskLevelNOMASK/2 +1;
			  priorityLevel_SysIrq %= IrqPriorityMaskLevelNOMASK;
			  xSetSysIrqPriorityLevel(SysIRQ_PendSV, priorityLevel_SysIrq);
	#endif
			  priorityLevel_SysIrq = xGetSysIrqPriorityLevel(SysIRQ_SysTick);
			  priorityLevel_SysIrq += IrqPriorityMaskLevelNOMASK/2 +1;
			  priorityLevel_SysIrq %= IrqPriorityMaskLevelNOMASK;
			  xSetSysIrqPriorityLevel(SysIRQ_SysTick, priorityLevel_SysIrq);

	#endif

	#if  0
			  // {{{ change IrqPriorityMaskLevel
			  uint32_t 	xIrqPriorityMaskLevel;

			  xIrqPriorityMaskLevel = xGetIrqPriorityMaskLevel();
			  xIrqPriorityMaskLevel += IrqPriorityMaskLevelNOMASK /2 +1;
			  xIrqPriorityMaskLevel %= IrqPriorityMaskLevelNOMASK +1;
			  setIrqPriorityMaskLevel(xIrqPriorityMaskLevel);
			  // }}}
	#endif

		  }

	  }
	#endif

	#if	0
	{
		static uint32_t	cnt_run = 0;

		if (++cnt_run > 400)
		{
		  uint32_t xCurrentMaskLevel = xGetIrqPriorityMaskLevel();
		  uint32_t xPriorityLevel = NVIC_GetPriority(SVCall_IRQn);

		  if (xPriorityLevel < xCurrentMaskLevel)
		  {
			  __ASM volatile("svc 0x8");
			  __ASM volatile("svc 0x7");
		  }
		  else if(xCurrentMaskLevel)
		  {
			  NVIC_SetPriority(SVCall_IRQn, xCurrentMaskLevel -1);
			  __ASM volatile("svc 0x8");
			  __ASM volatile("svc 0x7");
			  NVIC_SetPriority(SVCall_IRQn, xPriorityLevel);
		  }
		  else
		  {
			  // currently the IRQ is disable, nothing to do except to enable it
			  setIrqPriorityMaskLevel(IrqPriorityMaskLevelNOMASK);
			  __ASM volatile("svc 0x8");
			  __ASM volatile("svc 0x7");
			  setIrqPriorityMaskLevel(xCurrentMaskLevel);
		  }

		  cnt_run = 0;
		}
	}
	#endif

	#if	0
	  {     // test HAL_Tick
		  static const uint32_t	tick_c = 3000;
		  static uint32_t	tick_run = 0; //HAL_GetTick();
		  uint32_t    tick_current = HAL_GetTick();

		  if (tick_current > (tick_run + tick_c))
		  {
			  tick_run += tick_c;
			  BSP_LED_Toggle(LED4);
		  }
	  }
	#endif


	#if 0
	  {		// test RtosTick
		  static const TickType_t 	tick_c = 200;
		  static TickType_t 	tick_run = 0; // rtosGetRtosTick();
		  TickType_t    tick_currnt; // = rtosGetRtosTick();

		  tick_currnt = rtosGetRtosTick();
		  if (tick_currnt >  (tick_run + tick_c))
		  {
			  tick_run += tick_c;
			  BSP_LED_Toggle(LED6);
		  }
	  }
	#endif

#if 0
  {     // test RtosTick to release Sem
      static const TickType_t   tick_c = 100;
      static TickType_t     tick_run = 0; // rtosGetRtosTick();
      TickType_t    tick_currnt; // = rtosGetRtosTick();

      tick_currnt = rtosGetRtosTick();
      if (tick_currnt >  (tick_run + tick_c))
      {
          tick_run += tick_c;
          xSemRelease(theSem);
      }
  }
#endif

#if 0
  {     // test Commit  Run2Term Job  on a designed RtosTick period
      static const TickType_t   tick_c = 500;
      static TickType_t     tick_run = 0; // rtosGetRtosTick();
      TickType_t    tick_currnt; // = rtosGetRtosTick();

      tick_currnt = rtosGetRtosTick();
      if (tick_currnt >  (tick_run + tick_c))
      {
          tick_run += tick_c;
          BSP_LED_Toggle(LED_MAX);
          {
              static  JCB_t * pBlinkJCB = NULL;
          // submit a simple run2termination  job with shared_Stack
#if 0
              if(pBlinkJCB != NULL)
              {
                  xappendFreeListJCB(pBlinkJCB);
              }
              pBlinkJCB = xpickFreeListJCB();
#else
#if  01
              // manually force clear pBlinkJCB. Test RelocateToThreadStack or
              // FreeAtStartToRun, FreeAtTermination
              pBlinkJCB = NULL;
#endif

#endif

#if     0
              pBlinkJCB = rtos_commit_deferjob(NULL,
                                                          theLED4,
                                                          NULL,  ThreadStackTempStacking, 0 /*iStackSize*/,
                                                          blink_job_handler /*pJobHandler*/,
                                                          RTOS_HIGHEST_THREAD_PRIORITY /*xJobPriority*/,
                                                          RelocateToThreadStack,  01);
#endif
#if     0
              pBlinkJCB = rtos_commit_deferjob(NULL,
                                                          theLED4,
                                                          NULL,  ThreadStackTempStacking, 0 /*iStackSize*/,
                                                          blink_job_handler /*pJobHandler*/,
                                                          RTOS_HIGHEST_THREAD_PRIORITY /*xJobPriority*/,
                                                          RelocateToThreadStack,  50);
#endif
#if     0
              pBlinkJCB = rtos_commit_deferjob(NULL,
            		  LED_MAX +1,   //theLED3,
                                                          NULL,  ThreadStackTempStacking, 0 /*iStackSize*/,
                                                          blink_job_handler /*pJobHandler*/,
                                                          RTOS_HIGHEST_THREAD_PRIORITY /*xJobPriority*/,
                                                          RelocateToThreadStack,  40);
#endif
          }
      }
  }

#endif

	/* USER CODE END WHILE */

	/* USER CODE BEGIN 3 */
	  {
		  //static  JCB_t * pBlinkJCB = NULL;
		  //pBlinkJCB =
          rtos_commit_job(NULL,
                  LED_MAX ,   //theLED3,                             theLED3,
                  NULL,  ThreadStackTempStacking,
                  LED_BLINK_1SECONDDRIVE_JOB_NORM_STACK_SIZE /*iStackSize*/,
                  blink_1sencond_drive_job_handler /*pJobHandler*/,
                  LED_BLINK_1SECONDDRIVE_JOB_NORM_PRIORITY /*xJobPriority*/,
                  RelocateToThreadStack);

		  //pBlinkJCB =
                  rtos_commit_deferjob(NULL,
                  LED_MAX ,   //theLED3,                             theLED3,
                  NULL,  ThreadStackTempStacking,
                  LED_BLINK_1SECONDDRIVE_JOB_NORM_STACK_SIZE /*iStackSize*/,
                  blink_1sencond_drive_job_handler /*pJobHandler*/,
                  LED_BLINK_1SECONDDRIVE_JOB_NORM_PRIORITY /*xJobPriority*/,
                  RelocateToThreadStack,  100); // this delay gives lasting time period
	  }

	} while(0);

    // invoke idle_Job before return
    idle_Job_Handler(par);
    rtosSetRtosTickEnable(1);
  /* USER CODE END 3 */

    return 0;
}
/************************ (C) COPYRIGHT  *****END OF FILE****/
