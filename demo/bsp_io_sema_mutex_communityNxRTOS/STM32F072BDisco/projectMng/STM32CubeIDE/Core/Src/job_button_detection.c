/**
  ******************************************************************************
  * @file           : job_button_detection.c
  * @brief          :
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019
  * All rights reserved.</center></h2>
  *
  ******************************************************************************
  */

#include "rtos_jcb.h"
#include "bsp.h"
#include "job_button_detection.h"
#include "job_led_blink.h"
/// @param par
/// @return

static uint32_t state_BUTTON_USER;

int button_detect_init_job_handler(void * par)
{
	// BSP_PB_Init(BUTTON_MIN, BUTTON_MODE_RFEXTI);
	BSP_PB_Init(BUTTON_USER, BUTTON_MODE_RFEXTI);
	state_BUTTON_USER = BSP_PB_GetState(BUTTON_USER);
	return 0;
}

int button_detection_job_handler(void * par)
{
    //JCB_t * theJCB =(JCB_t *)par;
    uint32_t currentState_theButton;
    // (void)theJCB;

    currentState_theButton = BSP_PB_GetState(BUTTON_USER);
    if(state_BUTTON_USER !=  currentState_theButton)
    {	// update and reaction
    	state_BUTTON_USER =  currentState_theButton;
    	// send blink_job to act
    	rtos_commit_deferjob(NULL,
			  theLED3 ,   //theLED3,  theLED3,
			  NULL,  ThreadStackTempStacking,
			  LED_BLINK_JOB_NORM_STACK_SIZE /*iStackSize*/,
			  blink_job_handler /*pJobHandler*/,
			  LED_BLINK_JOB_NORM_PRIORITY /*xJobPriority*/,
			  RelocateToThreadStack,  60);
    }
    // return to terminate this thread.
    return 0;
}
