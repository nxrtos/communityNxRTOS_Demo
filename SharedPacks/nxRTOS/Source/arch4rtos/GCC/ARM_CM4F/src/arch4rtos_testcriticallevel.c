/* arch4rtos_testcriticallvel.c
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

/// {{{   TEST_SysCriticalLevel   {{{
#if     defined(TEST_SysCriticalLevel)
/// test from Thread_context
int xTestSysCriticalLevelFromThread()
{
  int result = 9;

  if(__get_IPSR())
  { // this test function has to invoke in Thread_context.
    while(1);
  }

  { // test arch4rtos_iGetSysCriticalLevel(void)
    __set_PRIMASK(1);
    while(HIGHEST_SYSCRITICALLEVEL != arch4rtos_iGetSysCriticalLevel());

    __set_PRIMASK(0);
    for(int i = 1; i < LOWEST_SYSCRITICALLEVEL; i++)
    {
      __set_BASEPRI(i << (8U - __NVIC_PRIO_BITS));
      while(i != arch4rtos_iGetSysCriticalLevel());
    }

    __set_BASEPRI(0);
    while(LOWEST_SYSCRITICALLEVEL != arch4rtos_iGetSysCriticalLevel());

    result--;
  } // test arch4rtos_iGetSysCriticalLevel(void)

  { // test arch4rtos_iSetSysCriticalLevel(newLevel)
    for(int i = HIGHEST_SYSCRITICALLEVEL; i <= LOWEST_SYSCRITICALLEVEL; i++)
    {
      while(i != arch4rtos_iSetSysCriticalLevel(i));
      while(i != arch4rtos_iGetSysCriticalLevel());
    }

    while(LOWEST_SYSCRITICALLEVEL !=
                    arch4rtos_iSetSysCriticalLevel(LOWEST_SYSCRITICALLEVEL +1));
    while(LOWEST_SYSCRITICALLEVEL != arch4rtos_iGetSysCriticalLevel());
    result--;
  } // test arch4rtos_iSetSysCriticalLevel(newLevel)

  // test arch4rtos_iRaiseSysCriticalLevel  and   xRestoreSysCriticalLevel
  for(int i = HIGHEST_SYSCRITICALLEVEL; i < LOWEST_SYSCRITICALLEVEL; i++)
  {
    while(i != arch4rtos_iSetSysCriticalLevel(i));
    while(i != arch4rtos_iGetSysCriticalLevel());
    while(i != arch4rtos_iRaiseSysCriticalLevel(i+1));
    while(i != arch4rtos_iGetSysCriticalLevel());
    while(i +1 != arch4rtos_iDropSysCriticalLevel(i +1));
    while(i +1 != arch4rtos_iGetSysCriticalLevel());
    if(i)   // i != 0
    {
      while(i +1 != arch4rtos_iRaiseSysCriticalLevel(i -1));
      while(i -1 != arch4rtos_iGetSysCriticalLevel());
      while(i +1 != arch4rtos_iDropSysCriticalLevel(i +1));
      while(i +1 != arch4rtos_iGetSysCriticalLevel());
    }
  }
  // test in ISR_context need through SVC_xx
  __ASM volatile("svc 0xFE");
  return result;
}

/// test from ISR_context
int xTestSysCriticalLevelFromISR()
{
  int result = 9;

  if(!__get_IPSR())
  {   // this test function has to invoke in ISR_context.
      while(1);
  }

  { // test arch4rtos_iGetSysCriticalLevel(void)
    __set_PRIMASK(0);
    __set_BASEPRI(0);

    for(int i = HIGHEST_SYSCRITICALLEVEL; i < LOWEST_SYSCRITICALLEVEL; i++ )
    {
      NVIC_SetPriority(__get_IPSR() -16, i);
      if(i != arch4rtos_iGetSysCriticalLevel())
      {
        while(1);
      }
    }

    __set_PRIMASK(1);
    if(HIGHEST_SYSCRITICALLEVEL != arch4rtos_iGetSysCriticalLevel())
    {
      while(1);
    }
    __set_PRIMASK(0);

    for(int i = HIGHEST_SYSCRITICALLEVEL +1; i < LOWEST_SYSCRITICALLEVEL; i++ )
    {
      __set_BASEPRI(i << (8U - __NVIC_PRIO_BITS));
        if(i != arch4rtos_iGetSysCriticalLevel())
        {
          while(1);
        }
    }

    __set_BASEPRI(0);
    if(LOWEST_SYSCRITICALLEVEL -1 != arch4rtos_iGetSysCriticalLevel())
    {
      while(1);
    }
  }
  return result;
}

#endif
/// }}}   TEST_SysCriticalLevel   }}}

