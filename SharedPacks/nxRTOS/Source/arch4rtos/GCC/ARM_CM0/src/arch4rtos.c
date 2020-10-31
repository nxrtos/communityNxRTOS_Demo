/*
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * 1 tab == 4 spaces!
 */
// removing
#if 0

/*-----------------------------------------------------------------------------
 * Implementation of functions defined in arch4rtos.h for the architecture of
 * ARM CM4F.
 * Refer to arch4rtos.tx for design consideration.
 *---------------------------------------------------------------------------*/
#include "arch4rtos.h"
#include "arch4rtos_ipl_mask.h"

// the initial value prevent any of pending Job to be executed.
// at the end of Os_Initialized, xSetThreadPriorityMaskLevel(configIdlePriority)
// will allow pending Jobs to executing based on priority order.
static uint32_t xCurrentThreadPriorityMaskLevel = 0x01 << __NVIC_PRIO_BITS;
uint32_t xGetThreadPriorityMaskLevel(void)
{
	return xCurrentThreadPriorityMaskLevel;
}


uint32_t xSetThreadPriorityMaskLevel(uint32_t newLevel)
{
	if(newLevel < IrqPriorityMaskLevelNOMASK)
	{
		newLevel = IrqPriorityMaskLevelNOMASK;
	}

	if(newLevel > configIdlePriority)
	{
		newLevel = configIdlePriority;
	}

	xCurrentThreadPriorityMaskLevel = newLevel;
	return newLevel;
}


// prototypes from core_cm0.h
/*	NVIC_GetPriority(IRQn_Type IRQn)
	NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority)

\brief   Set Interrupt Priority
\details Sets the priority of a device specific interrupt or a processor exception.
         The interrupt number can be positive to specify a device specific interrupt,
         or negative to specify a processor exception.
\param [in]      IRQn  Interrupt number.
\param [in]  priority  Priority to set. 0 to ((0x01<<__NVIC_PRIO_BITS) -1). un-shifted.
			 0 is default value from reset and highest priority.
			 ((0x01<<__NVIC_PRIO_BITS) -1) is lowest priority.
\note    The priority cannot be set for every processor exception.
*/
//uint32_t __NVIC_GetPriority(IRQn_Type IRQn)
//void __NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority)

// prototypes from cmsis_gcc.h
// Exception number in [5:0] , = IRQn + 16
//	uint32_t __get_IPSR();


/*
\brief   Get Current Context Priority Level
\details Gets the priority level on current context.
			if the API is invoked from an ISR, the current context will be the
			ISR. otherwise will be a thread context, and a thread priority
			level to be returned
*/
uint32_t xGetContextPriorityLevel(void * contextHandle)
{
	uint32_t xException, xPriorityLevel;

	if(!contextHandle)
	{	// =0 is a special contextHandle to current context
		xException = __get_IPSR();

		if(!xException)
		{ // current context is in thread mode
			xPriorityLevel = IrqPriorityMaskLevelNOMASK;
			// highest priority in thread mode

			// may invoke a generic RTOS Function to adjust to a lower number
		}
		else
		{ // current context is in ISR mode
			xPriorityLevel = NVIC_GetPriority(xException -16);
		}
	}
	else if((uint32_t)contextHandle < 255)
	{
		xException = (uint32_t)contextHandle;
		xPriorityLevel = NVIC_GetPriority(xException -16);
	}
	else
	{
		// look up contextHandle to find out
		// (pThread *)contextHandle -> xPriorityLevel;
		xPriorityLevel = IrqPriorityMaskLevelNOMASK;
	}


	return xPriorityLevel;
}

uint32_t xSetContextPriorityLevel(void * contextHandle, uint32_t newLevel)
{
	uint32_t xException, xPriorityLevel;

	if(!contextHandle)
	{	// =0 is a special contextHandle to current context
		xException = __get_IPSR();
		if(!xException)
		{ // current context is in thread mode
			if(newLevel >= IrqPriorityMaskLevelNOMASK)
			{
				// update current thread priority to newLevel.
				// xSetThreadPriorityLevel(0/*current thread*/,  newLevel)
				// some restriction may applied and result not exact newLevel
				// was assigned.
				xPriorityLevel = newLevel;
			}
			else
			{
				xPriorityLevel = IrqPriorityMaskLevelNOMASK;
			}
			// highest priority in thread mode

			// may invoke a generic RTOS Function to adjust to a lower number
		}
		else
		{ // current context is in ISR mode
			if(newLevel < IrqPriorityMaskLevelNOMASK)
			{
				NVIC_SetPriority(xException -16, newLevel);
				xPriorityLevel = newLevel;
			}
			else
			{
				xPriorityLevel = NVIC_GetPriority(xException -16);
			}
		}
	}
	else if((uint32_t)contextHandle < 255)
	{
		xException = (uint32_t)contextHandle;
		if(newLevel < IrqPriorityMaskLevelNOMASK)
		{
			NVIC_SetPriority(xException -16, newLevel);
		}	// newLeve is out range, get the original level as below

		// this is either set as newLevel by NVIC_SetPriority(...) above,
		// or the original level as before xSetContextPriorityLevel(...)
		xPriorityLevel = NVIC_GetPriority(xException -16);

	}
	else
	{
		// look up contextHandle to find out
		// (pThread *)contextHandle -> xPriorityLevel = newLevel;
		xPriorityLevel = IrqPriorityMaskLevelNOMASK;
	}

	return xPriorityLevel;

}
#endif
