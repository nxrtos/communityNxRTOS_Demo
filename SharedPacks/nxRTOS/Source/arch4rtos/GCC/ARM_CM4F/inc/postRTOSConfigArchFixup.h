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
/// for Cortex-M4 HW BASEPRI available, never need SOFTBASEPRI
#ifdef  configSOFTBASEPRI_Enable
#if   (configSOFTBASEPRI_Enable != 0)
#warning "no need SOFTBASEPRI on Cortex-M4 !!!"
//#define  SOFTBASEPRI_ENABLED    configSOFTBASEPRI_Enable
#undef SOFTBASEPRI_ENABLED
#else
#undef SOFTBASEPRI_ENABLED
#endif
#else
#undef SOFTBASEPRI_ENABLED
#endif
/// }}}  check  SOFTBASEPRI  Configuration   }}}

#if     defined(configRTOSCriticalLevel)
    #if     (configRTOSCriticalLevel < (0x01 << __NVIC_PRIO_BITS))
        #define     RTOS_SYSCRITICALLEVEL       configRTOSCriticalLevel
    #else
        #define     RTOS_SYSCRITICALLEVEL       ((0x01 << __NVIC_PRIO_BITS) -1)
    #endif
#endif

#endif /* _RTOSCONFIG_ARCH_FIXUP_H */
