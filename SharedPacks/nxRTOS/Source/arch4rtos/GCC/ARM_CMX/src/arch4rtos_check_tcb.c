/**
 ******************************************************************************
 * @file      arch4rtos_check_tcb.c
 * @author
 * @brief     check tcb and the contains in stack
 *
 ******************************************************************************
 * @attention
 */

#include  "arch4rtos_check_tcb.h"

void  arch4rtos_check_TCB(LiveTCB_t * theTCB)
{
  if(theTCB == NULL)
  {
    while(1);
  }
  else
  {
    StackType_t * sp = theTCB->pxTopOfStack;
    if(sp == NULL)
    {
      while(1);
    }
    else
    { // for cortex-M, the stack contains check
      // r4-r11, 8 regs
      StackType_t retValue = *(sp + 8);
      if( retValue != 0xfffffffd)
      {
        while(1);
      }
    }
  }
  return;
}
