/* memman_pool_type.h
 *
 * 1 tab == 4 spaces!
 */

/*
 * A sample implementation of pvPortMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of http://www.FreeRTOS.org for more information.
 */


#ifndef MEMMANPOOL_TYPE_H
#define MEMMANPOOL_TYPE_H


#include <stdlib.h>
#include <stdint.h>

#include "arch4rtos_basedefs.h"
//#include "projdefs.h"
//#include "portable.h"
//#include "syscritical_level.h"

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} BlockLink_t;

struct A_MEMPOOL_CONFIG;


typedef void * (* Func_Malloc)( const struct A_MEMPOOL_CONFIG * const p,size_t xWantedSize );

typedef void (* Func_Mfree)(const struct A_MEMPOOL_CONFIG * const pMemPoolConfig, void *pv );

/**
 * the configuration part
 */
// {

#define configTOTAL_HEAP_SIZE  	((size_t)3072)

#ifndef mtCOVERAGE_TEST_MARKER
	#define mtCOVERAGE_TEST_MARKER()
#endif

#ifndef traceMALLOC
    #define traceMALLOC( pvAddress, uiSize )
#endif

#ifndef traceFREE
    #define traceFREE( pvAddress, uiSize )
#endif


typedef struct A_MEMPOOL_CONFIG
{
    /// {{{  API  function  {{{
    Func_Malloc const   funcMalloc;
    Func_Mfree  const   funcFree;
    /// }}}  API  function  }}}

    /// {{{ common  initial data    {{{
	const	int		mem_pool_total_size;
	uint8_t * const	mem_pool_heap_start;
    /// }}} common  initial data    }}}

    /// {{{ atomic operation control {{{
	    //  Design choice , using arch4rtos_function to Mask SysCriticalLevel
	    //  aka. ISR_Level   to  prevent  concurrency.
	    //  the value usually shall between  Least_ISR_CriticalLevel and
	    //  configRTOSCriticalLevel.
	    //  And  usually a  configMemCriticalLevel  is provided.
	    // it is possible to use a unified type to combine
	    //  ISR_Critical and Thread_Critical and  use that for the op_priviledge.
	    //  and doing so means that Critical Control can be promote
	    //  Thread_Priority  rather than  ISR_Priority.
	    //  But bear in mind, lower this priviledge means more chances that
	    //  a mem_op may failed from a higher priority context when a
	    //  lower priority of context has ongoing mem_op on the mem_pool.
	    //  the ISR with priority more critical than configMemCriticalLevel is
	    //  NOT ALLOWED to invoke mem_op !!!!  neither malloc  nor  mfree  !!!
	const	SysCriticalLevel_t		mem_pool_op_priviledge;
    /// }}} atomic operation control  }}}

	const 	size_t 	mem_pool_struct_size;
	/* Work out the position of the top bit in a size_t variable. */
	//xBlockAllocatedBit = ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 );
	const 	size_t	mem_pool_blk_allocatedbit;

	uint8_t * const	mem_pool_op_lock;

	//const int     mem_pool_id;
	BlockLink_t * const   xpStart;
	BlockLink_t * * const ppxEnd;
	BlockLink_t * * const ppxPendingFree;

	size_t * const pxFreeBytesRemaining;
	size_t * const pxMinimumEverFreeBytesRemaining;

} MemPoolConfig_t;


extern 	// define	proto type function of AlgorithmFreeMerge_ mem pool management
void *	pvAlgorithmFreeMerge_Malloc(const MemPoolConfig_t * const pMemPoolConfig, size_t xWantedSize);
extern
void 	AlgorithmFreeMerge_Free(const MemPoolConfig_t * const pMemPoolConfig, void *pv);
extern
void *  pvAlgorithmFreeSimple_Malloc(const MemPoolConfig_t * const pMemPoolConfig, size_t xWantedSize);
extern
void    AlgorithmFreeSimple_Free(const MemPoolConfig_t * const pMemPoolConfig, void *pv);

// define mem op error code
#define MEM_OP_FAILED_STEAL			((void *)(-1))

#endif      //  MEMMANPOOL_TYPE_H
