/**
  ******************************************************************************
  * @file    CMSIS_Device.h
  * @author
  * @brief   CMSIS Device Peripheral Access Layer General Wrapper Header File.
  *
  ******************************************************************************
  * @attention
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/** @addtogroup CMSIS
  * @{  This is general wrapper for CMSIS_Device under construction.
  *     It relies on the Project Global Device Define to reach the proper
  *     CMSIS Device Header file supplied by semiconductor vender.
  *     Currently it implemented for STM32F0 and F4, may extend to
  *     support others in the future.
  */

#ifndef __CMSIS_DEVICE_H
#define __CMSIS_DEVICE_H

#if   0
#include "CMSIS_Device.txt.h"
#endif

// {{{  local version {{{
// this is a local copy from SharedPacks/CMSIS/Device/CMSIS_DeviceX.h
// with extra in beginning to help made a clean version
#include "stm32f0xx.h"

// make the def name in M0 consistent to M3/M4
#ifndef  SVCall_IRQn
  #define   SVCall_IRQn             SVC_IRQn
#endif

#include "stm32f051x8.h"
#include "core_cm0.h"             /* Cortex-M0 processor and core peripherals */
#include "system_stm32f0xx.h"
#include <stdint.h>

// }}}  local version }}}
//// after is wrapper for generic, no need when above local already provided.
// {{{  wrapper started   {{{
#if   0
#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */


 /**
   * @brief BLUEENG-X Family
   *////    {{{
#if defined(BLUENRG2_DEVICE)

#include "BlueNRG2.h"
 // make the def name in M0 consistent to M3/M4
#define SVC_IRQn        SVCall_IRQn
#endif
 ///  brief BLUEENG-X  }}}


/**
  * @brief STM32 Family
  */

 /**
   * @brief STM320 Family
   *////    {{{
#if defined(STM32F030x6)  || defined(STM32F030x8)  ||  defined(STM32F031x6) \
  ||  defined(STM32F038xx)  || defined(STM32F042x6)  || defined(STM32F048xx)  \
  ||  defined(STM32F051x8)  || defined(STM32F058xx)  || defined(STM32F070x6) \
  ||  defined(STM32F070xB)  ||  defined(STM32F071xB)  || defined(STM32F072xB) \
  ||  defined(STM32F078xx)  || defined(STM32F091xC)  ||  defined(STM32F098xx) \
  ||  defined(STM32F030xC)

#include "stm32f0xx.h"
 // make the def name in M0 consistent to M3/M4
#define     SVCall_IRQn             SVC_IRQn
#endif
 ///  brief STM320 Family  }}}

 /**
   * @brief STM32F4 Family
   *////    {{{
#if defined (STM32F405xx) || defined (STM32F415xx) || defined (STM32F407xx) \
    || defined (STM32F417xx) || defined (STM32F427xx) || defined (STM32F437xx) \
    || defined (STM32F429xx) || defined (STM32F439xx) || defined (STM32F401xC) \
    || defined (STM32F401xE) || defined (STM32F410Tx) || defined (STM32F410Cx) \
    || defined (STM32F410Rx) || defined (STM32F411xE) || defined (STM32F446xx) \
    || defined (STM32F469xx) || defined (STM32F479xx) || defined (STM32F412Cx) \
    || defined (STM32F412Rx) || defined (STM32F412Vx) || defined (STM32F412Zx) \
    || defined (STM32F413xx) ||defined (STM32F423xx)

#include "stm32f4xx.h"
#endif
 //   brief STM32F4 Family  }}}
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
// }}}  wrapper end   }}}

#endif /* __CMSIS_DEVICE_H */
/**
  * @}
  */

/**
  * @}
  */
  



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

