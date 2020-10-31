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

#include  "arch4rtos_testcriticallevel.h"

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
      // skip
    }

    while(LOWEST_SYSCRITICALLEVEL != arch4rtos_iGetSysCriticalLevel());

    result--;
  } // test arch4rtos_iGetSysCriticalLevel(void)

  { // test arch4rtos_iSetSysCriticalLevel(newLevel)
    while(HIGHEST_SYSCRITICALLEVEL !=
                  arch4rtos_iSetSysCriticalLevel(HIGHEST_SYSCRITICALLEVEL));
    while(HIGHEST_SYSCRITICALLEVEL != arch4rtos_iGetSysCriticalLevel());

    for(int i = 1; i < LOWEST_SYSCRITICALLEVEL; i++)
    {
      while(HIGHEST_SYSCRITICALLEVEL != arch4rtos_iSetSysCriticalLevel(i));
      while(HIGHEST_SYSCRITICALLEVEL != arch4rtos_iGetSysCriticalLevel());
    }

    while(LOWEST_SYSCRITICALLEVEL !=
                  arch4rtos_iSetSysCriticalLevel(LOWEST_SYSCRITICALLEVEL));
    while(LOWEST_SYSCRITICALLEVEL != arch4rtos_iGetSysCriticalLevel());

    while(LOWEST_SYSCRITICALLEVEL !=
                    arch4rtos_iSetSysCriticalLevel(LOWEST_SYSCRITICALLEVEL +1));
    while(LOWEST_SYSCRITICALLEVEL != arch4rtos_iGetSysCriticalLevel());
    result--;
  } // test arch4rtos_iSetSysCriticalLevel(newLevel)

  // test arch4rtos_iRaiseSysCriticalLevel  and   xRestoreSysCriticalLevel
  {
    int i = HIGHEST_SYSCRITICALLEVEL;
    while(i != arch4rtos_iSetSysCriticalLevel(i));
    while(i != arch4rtos_iGetSysCriticalLevel());
    while(i != arch4rtos_iRaiseSysCriticalLevel(i+1));
    while(i != arch4rtos_iGetSysCriticalLevel());
    while(i != arch4rtos_iDropSysCriticalLevel(i +1));
    while(i != arch4rtos_iGetSysCriticalLevel());
  }

  {
    int i = LOWEST_SYSCRITICALLEVEL;
    while(i != arch4rtos_iSetSysCriticalLevel(i));
    while(i != arch4rtos_iGetSysCriticalLevel());
    while(i != arch4rtos_iRaiseSysCriticalLevel(i+1));
    while(i != arch4rtos_iGetSysCriticalLevel());
    while(i != arch4rtos_iDropSysCriticalLevel(i +1));
    while(i != arch4rtos_iGetSysCriticalLevel());
  }

  for(int i = 1; i < LOWEST_SYSCRITICALLEVEL; i++)
  {

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

    for(int i = 1; i < LOWEST_SYSCRITICALLEVEL; i++ )
    {
    }

    NVIC_SetPriority(__get_IPSR() -16, LOWEST_ISRCRITICALLEVEL);

    if(LOWEST_SYSCRITICALLEVEL -1 != arch4rtos_iGetSysCriticalLevel())
    {
      while(1);
    }
  }
  return result;
}
/// }}}   TEST_SysCriticalLevel   }}}
#else
/// {{{   !TEST_SysCriticalLevel   {{{
int xTestSysCriticalLevelFromThread()
{
  return 0;
}
int xTestSysCriticalLevelFromISR()
{
  return 0;
}
#endif
/// }}}   !TEST_SysCriticalLevel   }}}
