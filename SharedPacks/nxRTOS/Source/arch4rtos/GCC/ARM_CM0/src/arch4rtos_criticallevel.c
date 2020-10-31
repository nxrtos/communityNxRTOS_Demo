/* arch4rtos_criticallevel.c
 *
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 2 spaces!
 */


/*-----------------------------------------------------------------------------
 * Implementation of SysCriticalLevel for the architecture of
 * ARM CM0.
 * Refer to arch4rtos.tx for design consideration.
 *---------------------------------------------------------------------------*/

#include  "arch4rtos_criticallevel.h"
#include  "nxRTOSConfig.h"

/// {{{     SysCriticalLevel_API    {{{
SysCriticalLevel_t arch4rtos_iGetSysCriticalLevel(void)
{
  SysCriticalLevel_t  theLevel;

  if(__get_PRIMASK())
  { // for PRIMASK = 1, straight HIGHEST_SYSCRITICALLEVEL
    theLevel = HIGHEST_SYSCRITICALLEVEL;
  }
  else  if(__get_IPSR())
  { // This is in an ISR context,
    theLevel = NVIC_GetPriority(__get_IPSR() -16);
  }
  else
  { // in Thread_context and PRIMASK == 0
    theLevel = LOWEST_SYSCRITICALLEVEL;
  }
  return theLevel;
}

/// in ISR_context, the new CriticalLevel can be set is between
/// HIGHEST_SYSCRITICALLEVEL and LOWEST_ISRCRITICALLEVEL
/// in Thread_context, the new CriticalLevel can be set is natively
/// only either HIGHEST_SYSCRITICALLEVEL (aka. Disable IRQ)
/// or THREAD_SYSCRITICALLEVEL.
///
/// @param newLevel
/// @returnbetween
SysCriticalLevel_t arch4rtos_iSetSysCriticalLevel(SysCriticalLevel_t newLevel)
{
  SysCriticalLevel_t  theLevel;

  //  sanity check.
  if(newLevel > LOWEST_SYSCRITICALLEVEL)
  {
    theLevel = LOWEST_SYSCRITICALLEVEL;
  }
  else
  {
    theLevel = newLevel;
  }

  if(theLevel != arch4rtos_iGetSysCriticalLevel())
  { // only try to set when current SysCriticalLevel != theLevel
    if(theLevel == HIGHEST_SYSCRITICALLEVEL)
    {
        __set_PRIMASK(1);
    }
    else if(theLevel == LOWEST_SYSCRITICALLEVEL)
    {
      __set_PRIMASK(0);
      if(__get_IPSR())
      { // in an ISR_context, with SOFTBASEPRI_DISABLLE, impossible to
        // LOWEST_SYSCRITICALLEVEL, try best to LOWEST_ISRCRITICALLEVEL
        theLevel = LOWEST_ISRCRITICALLEVEL;
        NVIC_SetPriority(__get_IPSR() -16, theLevel);
      }
      else
      {
        // already in Thread context, all set
      }
    }
    else  // for theLevel  between HIGHEST_SYSCRITICALLEVEL and
          // LOWEST_SYSCRITICALLEVEL, exclusively
    { // for theLevel  between HIGHEST_SYSCRITICALLEVEL and
      // LOWEST_SYSCRITICALLEVEL, exclusively
      if(__get_IPSR())
      { // in an ISR_context
        NVIC_SetPriority(__get_IPSR() -16, theLevel);
        __set_PRIMASK(0);
      }
      else
      { // original in Thread_context, can't set middle of SYSCRITICALLEVEL
        // without SOFTBASEPRI, set to HIGHEST_SYSCRITICALLEVEL instead
        theLevel = HIGHEST_SYSCRITICALLEVEL;
        __set_PRIMASK(1);
      }
    }
  }
  return theLevel;
}

SysCriticalLevel_t arch4rtos_iRaiseSysCriticalLevel(SysCriticalLevel_t newLevel)
{
  SysCriticalLevel_t  origLevel = arch4rtos_iGetSysCriticalLevel();

#  if   01
  while(newLevel <  arch4rtos_iGetSysCriticalLevel())
  // in some situation raising SysCriticalLevel may result to higher Level
  // than newLevel. So the check will be <
#else
  if (newLevel  < origLevel)
#endif
  {
    arch4rtos_iSetSysCriticalLevel(newLevel);
  }
  return origLevel;
}

// Decrease  SysCriticalLevel when possible
SysCriticalLevel_t arch4rtos_iDropSysCriticalLevel(SysCriticalLevel_t newLevel)
{
  SysCriticalLevel_t  theLevel;

  if(newLevel > LOWEST_SYSCRITICALLEVEL)
  {
      newLevel = LOWEST_SYSCRITICALLEVEL;
  }

  theLevel = arch4rtos_iGetSysCriticalLevel();
  if(newLevel > theLevel)
  { // only try to set newLevel be lower SysCriticalLevel
    theLevel = arch4rtos_iSetSysCriticalLevel(newLevel) ;
    // the result may not be newLevel as in ISR context
    // or with SOFTBASEPRI_DISABLED
    // sanity check the result may be little complicated.
  }
  return theLevel;
}
/// }}}     SysCriticalLevel_API    }}}
