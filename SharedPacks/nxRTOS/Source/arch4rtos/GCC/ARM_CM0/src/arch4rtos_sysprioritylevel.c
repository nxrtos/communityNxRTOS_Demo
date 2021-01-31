/* arch4rtos_sysprioritylevel.c
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
#if     0
/*-----------------------------------------------------------------------------
 * Implementation of functions defined in arch4rtos.h for the architecture of
 * ARM CM4F.
 * Refer to arch4rtos.tx for design consideration.
 *---------------------------------------------------------------------------*/
#include "arch4rtos.h"
#include "arch4rtos_ipl_mask.h"
#include "arch4rtos_criticallevel.h"
#include "list_tcb.h"

uint32_t xGetThreadPriorityLevel(void);
uint32_t xSetThreadPriorityLevel(uint32_t  newLevel);

// the initial value prevent any of pending Job to be executed.
// at the end of Os_Initialized, xSetThreadPriorityMaskLevel(configIdlePriority)
// will allow pending Jobs to executing based on priority order.

uint32_t xGetSysPriorityLevel(void)
{
	uint32_t theLevel;

	theLevel = arch4rtos_iGetSysCriticalLevel();

	if(theLevel == LOWEST_SYSCRITICALLEVEL)
	{
		theLevel = xGetThreadPriorityLevel();
	}

	return theLevel;
}

uint32_t xSetSysPriorityLevel(uint32_t newLevel)
{
	uint32_t theLevel;

	if(newLevel < LOWEST_SYSCRITICALLEVEL)
	{
	    theLevel = arch4rtos_iSetSysCriticalLevel(newLevel);
	}
	else
	{
	    theLevel = xSetThreadPriorityLevel(newLevel);
	}
	return theLevel;
}

__attribute__ (( weak ))
uint32_t xGetThreadPriorityLevel(void)
{
    uint32_t    thePriority;

    if(getCurrentTCB() != NULL)
    {
        thePriority = getCurrentTCB()->uxPriority;
    }
    else
    {
        thePriority = LOWEST_SYSCRITICALLEVEL; // aka Highest Thread_priorityLevel
    }

    return  thePriority;
}

__attribute__ (( weak ))
uint32_t xSetThreadPriorityLevel(uint32_t  newLevel)
{
    uint32_t    thePriority;

    thePriority = newLevel;

    if(thePriority < LOWEST_SYSCRITICALLEVEL)
    {
        thePriority = LOWEST_SYSCRITICALLEVEL;
    }

    if(getCurrentTCB() != NULL)
    {
        getCurrentTCB()->uxPriority = thePriority;
    }
    else
    {
        thePriority = LOWEST_SYSCRITICALLEVEL; // aka Highest Thread_priorityLevel
    }

    return  thePriority;
}
#endif
