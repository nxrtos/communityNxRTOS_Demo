/* arch4rtos_ipl_mask.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * Interrupt Priority Level Mask, no architecture support from CortexM0
 * 1 tab == 4 spaces!
 */


/*-----------------------------------------------------------------------------
 * Implementation of functions for Interrupt Request Priority Base Mask
 * on ARM CM0.
 * The idea is that the Interrupt Request with  Priority fall behind the
 * BaseMask will not to be serviced.
 * Because the ARM_CM0 does not have the BASEPRI which is on CM3/4,
 * this abstraction is not a good fit to ARM_CM0.
 * There is a more complicated implementation of SysCriticalLevel may used
 * rather than this Base Mask abstraction.
 *---------------------------------------------------------------------------*/

#include "arch4rtos_ipl_mask.h"
#include "nxRTOSConfig.h"

///     {{{
#ifdef        configIRQ_PRIORITY_BASE_MASK_SUPPORT

IrqPriorityBase_t   xGetIrqPriorityBase(void)
{
    uint32_t theLevel;

    if(__get_PRIMASK())
    {
        theLevel = 0;
    }
    else
    {   // for CM0, there is no BASEPRI, the MaskLevel can be
        // either defined as MaskLevelNOMASK,
        theLevel = IrqPriorityMaskLevelNOMASK;
    }
    return theLevel;
}

IrqPriorityBase_t   xSetIrqPriorityBase(IrqPriorityBase_t newLevel)
{

    if(!newLevel)
    {
        __set_PRIMASK(1);
    }
    else if(newLevel >= IrqPriorityMaskLevelNOMASK)
    {
        __set_PRIMASK(0);
        newLevel = IrqPriorityMaskLevelNOMASK;
    }
    else
    {   // for input    0< newLevel <IrqPriorityMaskLevelNOMASK
        // return current IrqPriorityMaskLevel, but no change
        if(__get_PRIMASK())
        {
            newLevel = 0;
        }
        else
        {   // for CM0, there is no BASEPRI
            newLevel = IrqPriorityMaskLevelNOMASK;
        }
    }

    return newLevel;
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

#endif      ///    configIRQ_PRIORITY_BASE_MASK_SUPPORT      }}}
