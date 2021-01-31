/* rtos_stackspace_longlive_thread.c
 * nxRTOS Kernel V0.0.1
 * Copyright (C) 2019  or its affiliates.  All Rights Reserved.
 *
 * 1 tab == 4 spaces!
 */

/*
 *  more reference to implementation of heap_4 in FreeRTOS for dedicated
 *  mem pool for Run2blck_Thread Stack Space.
 *  This can be implemented in highly configurable way !!!
 * */
#include  "rtos_stackspace.h"
#include  "new_stackspace4thread.h"
#include  "arch4rtos_criticallevel.h"
#include  "rtos_tcb_live_list.h"
#include  "nxRTOSConfig.h"

/// {{{ Longlive_Thread_StackSpace  {{{

#if     defined(RTOS_RUN2BLCK_STACKPOOLSIZE)
#define RTOS_RUN2BLCK_STACKPOOL_SizeInStackType             \
                ((RTOS_RUN2BLCK_STACKPOOLSIZE + sizeof(StackType_t) -1)  \
                                        / (sizeof(StackType_t)))
/// let's start with  predefined RTOS_RUN2BLCK_STACKSIZE
static StackType_t   theMemPool_4Run2BlckThreadStack
                                [RTOS_RUN2BLCK_STACKPOOL_SizeInStackType];

#endif


/// }}} Longlive_Thread_StackSpace  }}}


#if     defined(RTOS_RUN2BLCK_STACKPOOLSIZE)
/// {{{     support  RUN2BLCK_THREAD    {{{

#if    defined(STACK_GROW_TOLOWADDR)
static  StackType_t   *  run2blck_sp = &theMemPool_4Run2BlckThreadStack
								[RTOS_RUN2BLCK_STACKPOOL_SizeInStackType-1];
#else
static  StackType_t   *  run2blck_sp = &theMemPool_4Run2BlckThreadStack[0];
#endif

static  uint32_t    available_run2blck_size = RTOS_RUN2BLCK_STACKPOOLSIZE;

// get a RAM chunk for stack of a run_to_termination_thread
//
StackType_t * newStackSpace4Run2BlckThread(unsigned long  size)
{
	StackType_t * theSP = NULL;
    SysCriticalLevel_t origCriticalLevel = arch4rtos_iGetSysCriticalLevel();

    // {{{ critical section enter {{{
    arch4rtos_iRaiseSysCriticalLevel(RTOS_SYSCRITICALLEVEL);

    // allocate from run2blck_stack pool
    if(available_run2blck_size > size + sizeof(R2BTCB_t))
    {
        theSP = run2blck_sp;

	// performing alignment.
#if    defined(STACK_GROW_TOLOWADDR)
    {   // make room for TCB
        theSP = (StackType_t *)((AddressOpType_t)theSP - sizeof(R2BTCB_t));
        // Alignment to lower address,
        theSP = (StackType_t *)
                (((AddressOpType_t) theSP) >>  STACK_ALIGNMENT_BITS);
        theSP = (StackType_t *)
                (((AddressOpType_t) theSP) <<  STACK_ALIGNMENT_BITS);
	}
#else   //  STACK_GROW_TOHIGHADDR
	{   // make room for TCB
        theSP = (void *)theSP  +  sizeof(R2BTCB_t);
        // Alignment to higher address,
        theSP = (StackType_t *)  (((StackType_t )theSP) + ((1 << STACK_ALIGNMENT_BITS) -1) );
        theSP = (StackType_t *)  (((StackType_t )theSP) >>  STACK_ALIGNMENT_BITS);
        theSP = (StackType_t *)  (((StackType_t )theSP) <<  STACK_ALIGNMENT_BITS);
	}
#endif
    }
    else
    {
    }


#if    defined(STACK_GROW_TOLOWADDR)
    if(theSP - size > &theMemPool_4Run2BlckThreadStack[0])
    {   // success
        run2blck_sp = theSP - size;
        available_run2blck_size = run2blck_sp -
                                        &theMemPool_4Run2BlckThreadStack[0];
    }
#else
    if(theSP + size < &theMemPool_4Run2BlckThreadStack[RTOS_RUN2BLCK_STACKPOOL_SizeInStackType -1])
    {   // success
        run2blck_sp = theSP + size;
        available_run2blck_size = &theMemPool_4Run2BlckThreadStack[RTOS_RUN2BLCK_STACKPOOL_SizeInStackType -1] - run2blck_sp;
    }
#endif
    else
    {   // no enough space left
        theSP = NULL;
    }
    //  }}}     critical section exit    }}}
    arch4rtos_iDropSysCriticalLevel(origCriticalLevel);
	return  theSP;
}

#endif  ///  }}}    RUN2BLCK_THREAD }}}
