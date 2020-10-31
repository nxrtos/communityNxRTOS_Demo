/*
 * nxRTOS Kernel V0.0.1		postRTOSConfigArchFixup.h
 * Copyright (C) 2019 . or its affiliates.  All Rights Reserved.
 *
 *
 * 1 tab == 4 spaces!
 */


#ifndef  _RTOSCONFIG_ARCH_FIXUP_H
#define  _RTOSCONFIG_ARCH_FIXUP_H

/// {{{  check  SOFTBASEPRI  Configuration   {{{
#ifdef  configSOFTBASEPRI_Enable
#undef SOFTBASEPRI_ENABLED
#if   (configSOFTBASEPRI_Enable != 0)
//#define  SOFTBASEPRI_ENABLED    configSOFTBASEPRI_Enable
#warning "community version does not support SOFTBASEPRI"
#else
#undef SOFTBASEPRI_ENABLED
#endif
#else
#undef SOFTBASEPRI_ENABLED
#endif


#if defined(SOFTBASEPRI_ENABLED)
  #if     defined(configRTOSCriticalLevel)
    #if     (configRTOSCriticalLevel < (0x01 << __NVIC_PRIO_BITS))
      #define     RTOS_SYSCRITICALLEVEL       configRTOSCriticalLevel
    #else
      #define     RTOS_SYSCRITICALLEVEL       ((0x01 << __NVIC_PRIO_BITS) -1)
    #endif
  #else
    #define     RTOS_SYSCRITICALLEVEL       ((0x01 << __NVIC_PRIO_BITS) /2)
  #endif
#else // undefined(SOFTBASEPRI_ENABLED) RTOS_SYSCRITICALLEVEL has to be 0
  #define     RTOS_SYSCRITICALLEVEL 0
#endif

/// }}}  check  SOFTBASEPRI  Configuration   }}}

#endif /* _RTOSCONFIG_ARCH_FIXUP_H */
