/* arch4rtos_ipl_mask.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 *
 * 1 tab == 4 spaces!
 */


/*-----------------------------------------------------------------------------
 * Implementation of functions for Interrupt Request Priority Base Mask
 * on ARM CM4F.
 * The Interrupt Request Priority fall behind the BaseMask
 * will not to be serviced
 *---------------------------------------------------------------------------*/

#include "arch4rtos_ipl_mask.h"
#include "nxRTOSConfig.h"

IrqPriorityBase_t   xGetIrqPriorityBase(void)
{
	uint32_t theLevel;

	if(__get_PRIMASK())
	{
		theLevel = IrqPriorityMaskLevelHIGHEST;
	}
	else
	{
		theLevel = __get_BASEPRI();
		// BASEPRI is 8 bit size defined by ARM.
		// the high __NVIC_PRIO_BITS bits field are implemented by vender,
		// and lower are zeroed out.
		if(!theLevel)
		{
			theLevel = IrqPriorityMaskLevelNOMASK;
		}
		else
		{
			theLevel >>= 8U - __NVIC_PRIO_BITS;
		}
	}

	return theLevel;
}

IrqPriorityBase_t   xSetIrqPriorityBase(IrqPriorityBase_t newLevel)
{
	uint32_t theLevel;

	theLevel = newLevel;
	if(theLevel)
	{
		if ( theLevel < IrqPriorityMaskLevelNOMASK)
		{
			__set_BASEPRI(newLevel << (8U - __NVIC_PRIO_BITS));
		}
		else
		{
			theLevel = IrqPriorityMaskLevelNOMASK;
			__set_BASEPRI(0);
		}
		// check ARM DUI 0553B (ID012616)
		// <<Cortex-M4 DevicesGeneric User Guide>>   2.1.3  Core registers
		//
		__set_PRIMASK(0);
	}
	else
	{	// PRIMASK 0 = no effect
		// 1 = prevents the activation of all exceptions with configurable priority.
		__set_PRIMASK(1);
	}

	return theLevel;
}

IrqPriorityBase_t   xRaiseIrqPriorityBase(IrqPriorityBase_t newLevel)
{
    if(newLevel < xGetIrqPriorityBase())
    {
        xSetIrqPriorityBase(newLevel);
    }
    return xGetIrqPriorityBase();
}

IrqPriorityBase_t   xDropIrqPriorityBase(IrqPriorityBase_t newLevel)
{
    if(newLevel > xGetIrqPriorityBase())
    {
        xSetIrqPriorityBase(newLevel);
    }
    return xGetIrqPriorityBase();
}
