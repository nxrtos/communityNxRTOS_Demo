/**
  ******************************************************************************
  * File Name          : bsp.h
  * Description        :
  ******************************************************************************
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_H
#define __BSP_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0308_discovery.h"
#include "stm32f0xx_hal.h"
// it is no need blow as  "stm32f0xx_hal_conf.h" will pick up right option,
// but here for eliminate compiler false warning/error
#include "stm32f0xx_hal_rcc.h"
#include "stm32f0xx_hal_flash.h"
#include "stm32f0xx_hal_cortex.h"
#include "stm32f0xx_hal_gpio.h"
/**
  * @}
  */ 

/**
  * @}
*/

#endif /* __BSP_H */
