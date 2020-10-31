/**
  ******************************************************************************
  * @file           : blink_job.c
  * @brief          :
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */

#include "job_led_blink.h"
#include "bsp.h"

/// @param par
/// @return
int blink_job_handler(void * par)
{
    JCB_t * theJCB =(JCB_t *)par;
    // (void)theJCB;

    if(theJCB->iJobPar <= LED_MAX)
    {
    	BSP_LED_Toggle(theJCB->iJobPar);
    }

    // return to terminate this thread.
    return 0;
}


int blink_1sencond_drive_job_handler(void * par)
{
    JCB_t * theJCB =(JCB_t *)par;
    // (void)theJCB;

    if(theJCB->iJobPar <= LED_MAX)
    {
    	BSP_LED_Toggle(theJCB->iJobPar);
        // repeat itself
        rtos_commit_deferjob(NULL,
              theJCB->iJobPar ,   //theLED3,                             theLED3,
              NULL,  ThreadStackTempStacking,
			  LED_BLINK_1SECONDDRIVE_JOB_NORM_STACK_SIZE /*iStackSize*/,
              blink_1sencond_drive_job_handler /*pJobHandler*/,
              LED_BLINK_1SECONDDRIVE_JOB_NORM_PRIORITY /*xJobPriority*/,
			  RelocateToThreadStack,  1000);
    }

    // return to terminate this thread.
    return 0;
}
