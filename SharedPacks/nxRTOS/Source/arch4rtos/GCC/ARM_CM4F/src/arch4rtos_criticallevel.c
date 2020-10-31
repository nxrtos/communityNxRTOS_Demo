/* arch4rtos_criticallevel.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */
/*-----------------------------------------------------------------------------
 * Implementation of SysCriticalLevel for the architecture of
 * ARM CM4F.
 * Refer to arch4rtos.tx for design consideration.
 *---------------------------------------------------------------------------*/

#include  "arch4rtos_criticallevel.h"

SysCriticalLevel_t arch4rtos_iGetSysCriticalLevel(void)
{
  SysCriticalLevel_t  theLevel;

  if(__get_PRIMASK())
  {	// for PRIMASK = 1, straight HIGHEST_SYSCRITICALLEVEL
    theLevel = HIGHEST_SYSCRITICALLEVEL;
  }
  else if(__get_IPSR())
  { // This is in an ISR context,
    if(__get_BASEPRI())
    { // for BASEPRI != 0,  the SysCriticalLevel is
      // the lower from BASEPRI and NVIC_Priority[ISR_Num]
      // ISR_Number = Exception_Number -16
      theLevel = __get_BASEPRI() >> (8U - __NVIC_PRIO_BITS)
                     <  NVIC_GetPriority(__get_IPSR() -16)
                 ?  __get_BASEPRI() >>(8U - __NVIC_PRIO_BITS)
                    :
                    NVIC_GetPriority(__get_IPSR() -16);
    }
    else
    { // in ISR_context  and BASEPRI = 0,the SysCriticalLevel is
      theLevel = NVIC_GetPriority(__get_IPSR() -16);
    }
    // if(theLevel = (__get_BASEPRI() >>(8U - __NVIC_PRIO_BITS));)
  }
  else
  { // in Thread_context and PRIMASK == 0
    if(__get_BASEPRI())
    {
      theLevel = __get_BASEPRI() >> (8U - __NVIC_PRIO_BITS);
    }
    else
    {
      theLevel = LOWEST_SYSCRITICALLEVEL;
    }
  }
  return theLevel;
}

SysCriticalLevel_t arch4rtos_iSetSysCriticalLevel(SysCriticalLevel_t newLevel)
{
  SysCriticalLevel_t  theLevel;

  if(newLevel >= LOWEST_SYSCRITICALLEVEL)
  { // try to lowest possible
    if(__get_IPSR())
    { //  to set SysCriticalLevel to LOWEST_SYSCRITICALLEVEL in
      // ISR_context is impossible, only in a special extension software
      // feature enabled.
      NVIC_SetPriority(__get_IPSR() -16, LOWEST_ISRCRITICALLEVEL);
      theLevel = LOWEST_ISRCRITICALLEVEL;
    }
    else
    {
      theLevel = LOWEST_SYSCRITICALLEVEL;
    }
    __set_BASEPRI(0);
    __set_PRIMASK(0);
  }
  else if(newLevel == HIGHEST_SYSCRITICALLEVEL)
  {
    if(__get_IPSR() && (newLevel == NVIC_GetPriority(__get_IPSR() -16)))
    {
      ;
    }
    else
    { // simply set PRIMASK 1 to reach HIGHEST_SYSCRITICALLEVEL
      __set_PRIMASK(1);
    }
    theLevel = HIGHEST_SYSCRITICALLEVEL;
  }
  else
  { // newLevel is in between  HIGHEST_SYSCRITICALLEVEL and
    // LOWEST_SYSCRITICALLEVEL, exclude them
    if(__get_IPSR())
    { //  to set SysCriticalLevel to LOWEST_SYSCRITICALLEVEL in
      // ISR_context is impossible, only in a special extension software
      // feature enabled.
      if(newLevel == NVIC_GetPriority(__get_IPSR() -16))
      { // for blocking level is same as NVIC_ISR entry Priority,
        // simply clear BASEPRI
        __set_BASEPRI(0);
      }
      else if(newLevel < NVIC_GetPriority(__get_IPSR() -16))
      { // when need to raise blocking level than NVIC_ISR entry Priority,
        // set BASEPRI
        __set_BASEPRI(newLevel << (8U - __NVIC_PRIO_BITS));
      }
      else if(newLevel > NVIC_GetPriority(__get_IPSR() -16))
      { // when need to lower blocking level than NVIC_ISR entry Priority,
        // set NVIC_ISRPriority and clear BASEPRI
        NVIC_SetPriority(__get_IPSR() -16, newLevel);
        __set_BASEPRI(0);
      }
    }
    else
    { // in Thread_context to set CriticalLevel to newLevel
      // between HIGHEST_SYSCRITICALLEVEL and
      // LOWEST_SYSCRITICALLEVEL  but exclude them
      __set_BASEPRI(newLevel << (8U - __NVIC_PRIO_BITS));
    }
    theLevel = newLevel;
    // make sure to clear PRIMASK when set  SYSCRITICALLEVEL
    // between  HIGHEST_    and LOWEST_SYSCRITICALLEVEL
    __set_PRIMASK(0);
  }
  return theLevel;
}

// most use  arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL)

// return original SysCriticalLevel. new Level can get from
// arch4rtos_iGetSysCriticalLevel
SysCriticalLevel_t arch4rtos_iRaiseSysCriticalLevel(SysCriticalLevel_t newLevel)
{
  SysCriticalLevel_t  origLevel = arch4rtos_iGetSysCriticalLevel();

  if(newLevel > LOWEST_SYSCRITICALLEVEL)
  {
    newLevel = LOWEST_SYSCRITICALLEVEL;
  }

  if(newLevel < origLevel)
  { // act only when need to raise
    do
    {
      arch4rtos_iSetSysCriticalLevel(newLevel);
    } // do sanity check
    while(newLevel != arch4rtos_iGetSysCriticalLevel());
    // while(0);
  }

  return origLevel;
}

// Drop SysCriticalLevel when possible
SysCriticalLevel_t arch4rtos_iDropSysCriticalLevel(SysCriticalLevel_t newLevel)
{
  SysCriticalLevel_t  theLevel = arch4rtos_iGetSysCriticalLevel();

  if(newLevel > LOWEST_SYSCRITICALLEVEL)
  {
    newLevel = LOWEST_SYSCRITICALLEVEL;
  }

  if(newLevel > theLevel)
  { // action only when need to drop SysCriticalLevel
    do
    {
      arch4rtos_iSetSysCriticalLevel(newLevel);
    } // do sanity check, while(0) otherwise
    while(newLevel != arch4rtos_iGetSysCriticalLevel());
    // while(0);
    theLevel = newLevel;
  }

  return theLevel;
}
