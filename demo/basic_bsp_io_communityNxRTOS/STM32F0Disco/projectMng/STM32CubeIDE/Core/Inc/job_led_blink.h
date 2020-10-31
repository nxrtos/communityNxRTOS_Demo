/**
  ******************************************************************************
  * @file           : job_led_blink.h
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
#ifndef    _JOB_LED_BLINK_H
#define    _JOB_LED_BLINK_H

#include    "stdint.h"
#include    "CMSIS_Device.h"
#include    "rtos_commit_job.h"

typedef     uint16_t        LDX_Pin_t;
typedef     GPIO_TypeDef *      LDX_GPIO_Port;

typedef     struct      __led_desc_st
{
    LDX_GPIO_Port   port;
    LDX_Pin_t               pin;
}   LED_DESC_t;

typedef     enum    _led_enum_
{
    theLED3,
    orangeLED = theLED3,
    theLED4,
    greenLED = theLED4,
    theLED5,
    redLED = theLED5,
    theLED6,
    blueLED = theLED6,
    bdrLED
}LED_t;

//  use 3 bit to accommodate  theLED presenting
#define     theLED_NumBits                                  3
#define     theLED_BitMask                                  ((1 << theLED_NumBits) -1)

#ifdef __cplusplus
extern "C" {
#endif

extern  int  blink_job_handler(void * par);
extern  int  blink_1sencond_drive_job_handler(void * par);

#define  LED_BLINK_JOB_NORM_PRIORITY	RTOS_NORMAPP_THREAD_PRIORITY
#define  LED_BLINK_JOB_NORM_STACK_SIZE	0x400 /* use RTOS_MINIMAL_THREAD_STACKSIZE */

#define  LED_BLINK_1SECONDDRIVE_JOB_NORM_PRIORITY	RTOS_NORMAPP_THREAD_PRIORITY
#define  LED_BLINK_1SECONDDRIVE_JOB_NORM_STACK_SIZE	0x400 /* THREAD_STACKSIZE */

#ifdef __cplusplus
}
#endif

#endif  //_JOB_LED_BLINK_H
