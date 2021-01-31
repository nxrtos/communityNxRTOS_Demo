/*
 *
 * 1 tab == 2 spaces!
 */

/*
 * A sample implementation of pvPortMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of http://www.FreeRTOS.org for more information.
 */

#include <stdlib.h>
#include <stdint.h>

#include "memman_pool_type.h"
#include "arch4rtos_basedefs.h"
#include "arch4rtos_criticallevel.h"
//#include "portable.h"

#include "nxRTOSConfig.h"


// this may defined in "portmacro.h" from "portable.h"
#ifndef	heapBITS_PER_BYTE
/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE              ( ( size_t ) 8 )
#endif

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList	// ();
	(const MemPoolConfig_t * const pMemPoolConfig, BlockLink_t *pxBlockToInsert );

static void prvAddBlockIntoPendingFreeList	// ();
	(const MemPoolConfig_t * const pMemPoolConfig, BlockLink_t *pxBlockToAdd );
static BlockLink_t * prvPickBlockFromPendingFreeList	// ();
								(const MemPoolConfig_t * const pMemPoolConfig);

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit( const MemPoolConfig_t * const pMemPoolConfig );

/*-----------------------------------------------------------*/


void *pvAlgorithmFreeMerge_Malloc( const MemPoolConfig_t * const pMemPoolConfig,size_t xWantedSize )
{
BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
void *pvReturn = NULL;
SysCriticalLevel_t	xOrgCrtLevel = arch4rtos_iGetSysCriticalLevel();

	//vTaskSuspendAll(); guarantee  op_critical not below to mem_pool_op_priviledge
	// and set mem_pool_op_lock properly

    /// {{{ SysCriticalLevel    {{{
    arch4rtos_iRaiseSysCriticalLevel(pMemPoolConfig->mem_pool_op_priviledge);
    if(*(pMemPoolConfig->mem_pool_op_lock))
    {   // the op_lock is locked by a lower SysCriticalLevel  context.
        // pvReturn = MEM_OP_FAILED_STEAL;
        // if all agree this special RETURN other than NULL
        arch4rtos_iDropSysCriticalLevel(xOrgCrtLevel);
        return pvReturn;
    }
    else
    {   // set the op_lock.  seems this set no need to have SysCriticalLevel protection.
        *(pMemPoolConfig->mem_pool_op_lock) = 0x1;
    }

	{	/* If this is the first call to malloc then the heap will require
		initialisation to setup the list of free blocks. */
		if( *(pMemPoolConfig->ppxEnd) == NULL )
		{
			prvHeapInit(pMemPoolConfig);
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}

	{	/* Check the requested block size is not so large that the top bit is
		set.  The top bit of the block size member of the BlockLink_t structure
		is used to determine who owns the block - the application or the
		kernel, so it must be free. */
		if( ( xWantedSize & (pMemPoolConfig->mem_pool_blk_allocatedbit) ) == 0 )
		{
			/* The wanted size is increased so it can contain a BlockLink_t
			structure in addition to the requested amount of bytes. */
			if( xWantedSize > 0 )
			{
				xWantedSize += pMemPoolConfig->mem_pool_struct_size;

				/* Ensure that blocks are always aligned to the required number
				of bytes. */
				if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
				{
					/* Byte alignment required. */
					xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
//					configASSERT( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) == 0 );
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}

			if( ( xWantedSize > 0 ) && ( xWantedSize <= *(pMemPoolConfig->pxFreeBytesRemaining) ) )
			{
				/* Traverse the list from the start	(lowest address) block until
				one	of adequate size is found. */
				pxPreviousBlock = pMemPoolConfig->xpStart;
				pxBlock = pMemPoolConfig->xpStart->pxNextFreeBlock;
				while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
				{
					pxPreviousBlock = pxBlock;
					pxBlock = pxBlock->pxNextFreeBlock;
				}

				/* If the end marker was reached then a block of adequate size
				was	not found. */
				if( pxBlock != *(pMemPoolConfig->ppxEnd) )
				{
					/* Return the memory space pointed to - jumping over the
					BlockLink_t structure at its start. */
					pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNextFreeBlock ) + pMemPoolConfig->mem_pool_struct_size );

					/* This block is being returned for use so must be taken out
					of the list of free blocks. */
					pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

					/* If the block is larger than required it can be split into
					two. */
					if( ( pxBlock->xBlockSize - xWantedSize ) > // heapMINIMUM_BLOCK_SIZE
											((size_t)(pMemPoolConfig->mem_pool_struct_size << 1)) )
					{
						/* This block is to be split into two.  Create a new
						block following the number of bytes requested. The void
						cast is used to prevent byte alignment warnings from the
						compiler. */
						pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );
//						configASSERT( ( ( ( size_t ) pxNewBlockLink ) & portBYTE_ALIGNMENT_MASK ) == 0 );

						/* Calculate the sizes of two blocks split from the
						single block. */
						pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
						pxBlock->xBlockSize = xWantedSize;

						/* Insert the new block into the list of free blocks. */
						prvInsertBlockIntoFreeList( pMemPoolConfig,pxNewBlockLink );
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}

					*(pMemPoolConfig->pxFreeBytesRemaining) -= pxBlock->xBlockSize;

					if( *(pMemPoolConfig->pxFreeBytesRemaining) <
											*(pMemPoolConfig->pxMinimumEverFreeBytesRemaining) )
					{
						*(pMemPoolConfig->pxMinimumEverFreeBytesRemaining) = *(pMemPoolConfig->pxFreeBytesRemaining);
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}

					/* The block is being returned - it is allocated and owned
					by the application and has no "next" block. */
					pxBlock->xBlockSize |= (pMemPoolConfig->mem_pool_blk_allocatedbit);
					pxBlock->pxNextFreeBlock = NULL;
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		traceMALLOC( pvReturn, xWantedSize );
	}


	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if( pvReturn == NULL )
		{
			extern void vApplicationMallocFailedHook( void );
			vApplicationMallocFailedHook();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	#endif

//	configASSERT( ( ( ( size_t ) pvReturn ) & ( size_t ) portBYTE_ALIGNMENT_MASK ) == 0 );

	do	// check PendingFreeList before unlock and restore SysCriticalLevel
	{
		pxBlock = prvPickBlockFromPendingFreeList(pMemPoolConfig);
		if(pxBlock != NULL)
		{
			/* The block is being returned to the heap - it is no longer
			allocated. */
			pxBlock->xBlockSize &= ~(pMemPoolConfig->mem_pool_blk_allocatedbit);
			/* Add this block to the list of free blocks. */
			*(pMemPoolConfig->pxFreeBytesRemaining) += pxBlock->xBlockSize;
			traceFREE( pv, pxBlock->xBlockSize );
			prvInsertBlockIntoFreeList( pMemPoolConfig, pxBlock );

		}
	}while(pxBlock != NULL);

	//	( void ) xTaskResumeAll(); release lock before return
	{
		*(pMemPoolConfig->mem_pool_op_lock) = 0;
		arch4rtos_iDropSysCriticalLevel(xOrgCrtLevel);
	}   /// }}}  SysCriticalLevel    }}}

	return pvReturn;
}
/*-----------------------------------------------------------*/

void AlgorithmFreeMerge_Free(const MemPoolConfig_t * const pMemPoolConfig, void *pv )
{
uint8_t *puc = ( uint8_t * ) pv;
BlockLink_t *pxLink;

	if( pv != NULL )
	{
		/* The memory being freed will have an BlockLink_t structure immediately
		before it. */
		puc -= pMemPoolConfig->mem_pool_struct_size;

		/* This casting is to keep the compiler from issuing warnings. */
		pxLink = (BlockLink_t *) puc;

		/* Check the block is actually allocated. */
//		configASSERT( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 );
//		configASSERT( pxLink->pxNextFreeBlock == NULL );

		if( ( pxLink->xBlockSize & (pMemPoolConfig->mem_pool_blk_allocatedbit) ) != 0 )
		{
			if( pxLink->pxNextFreeBlock == NULL )
			{
				// lock op before any real change

				SysCriticalLevel_t	xOrgCrtLevel = arch4rtos_iGetSysCriticalLevel();

			    /// {{{ SysCriticalLevel    {{{
			    arch4rtos_iRaiseSysCriticalLevel(pMemPoolConfig->mem_pool_op_priviledge);

                if(*(pMemPoolConfig->mem_pool_op_lock))
                {	// failed to steal lock, put pv into a pending list for later processing

                    // TODO, add pv to pending_to_Free list,
                    // as currently  ->mem_pool_op_lock locked by others
                    // prvInsertBlockIntoFreeList( pMemPoolConfig, pv )???
                    //(void)pv;
                    prvAddBlockIntoPendingFreeList( pMemPoolConfig, pxLink);
                    arch4rtos_iDropSysCriticalLevel(xOrgCrtLevel);
                    return;
                }
				else
                {
                    *(pMemPoolConfig->mem_pool_op_lock) = 0x01;
                    //continue op on mem_pool_op_priviledge or above
                }

				/* The block is being returned to the heap - it is no longer
				allocated. */
				pxLink->xBlockSize &= ~(pMemPoolConfig->mem_pool_blk_allocatedbit);

//				vTaskSuspendAll();
				{
					/* Add this block to the list of free blocks. */
					*(pMemPoolConfig->pxFreeBytesRemaining) += pxLink->xBlockSize;
					traceFREE( pv, pxLink->xBlockSize );
					prvInsertBlockIntoFreeList( pMemPoolConfig, ( ( BlockLink_t * ) pxLink ) );
				}
//				( void ) xTaskResumeAll();

				do	// check PendingFreeList before unlock and restore SysCriticalLevel
				{
					pxLink = prvPickBlockFromPendingFreeList(pMemPoolConfig);
					if(pxLink != NULL)
					{
						/* The block is being returned to the heap - it is no longer
						allocated. */
						pxLink->xBlockSize &= ~(pMemPoolConfig->mem_pool_blk_allocatedbit);
						/* Add this block to the list of free blocks. */
						*(pMemPoolConfig->pxFreeBytesRemaining) += pxLink->xBlockSize;
						traceFREE( pv, pxLink->xBlockSize );
						prvInsertBlockIntoFreeList( pMemPoolConfig, ( ( BlockLink_t * ) pxLink ) );

					}
				}while(pxLink != NULL);

				{	// unclok ->mem_pool_op_lock and restore SysCriticalLevel
					*(pMemPoolConfig->mem_pool_op_lock) = 0;
					arch4rtos_iDropSysCriticalLevel(xOrgCrtLevel);
				}   /// }}} SysCriticalLevel    }}}

			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
}
/*-----------------------------------------------------------*/

// TO be removed {
/*-----------------------------------------------------------*/
// TO be removed }

static void prvHeapInit( const MemPoolConfig_t * const pMemPoolConfig )
{
BlockLink_t *pxFirstFreeBlock;
uint8_t *pucAlignedHeap;
size_t uxAddress;
size_t xTotalHeapSize = pMemPoolConfig->mem_pool_total_size;// configTOTAL_HEAP_SIZE;

	/* Ensure the heap starts on a correctly aligned boundary. */
	uxAddress = ( size_t ) pMemPoolConfig->mem_pool_heap_start;

	if( ( uxAddress & portBYTE_ALIGNMENT_MASK ) != 0 )
	{
		uxAddress += ( portBYTE_ALIGNMENT - 1 );
		uxAddress &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
		xTotalHeapSize -= uxAddress - ( size_t ) pMemPoolConfig->mem_pool_heap_start;
	}

	pucAlignedHeap = ( uint8_t * ) uxAddress;

	/* xStart is used to hold a pointer to the first item in the list of free
	blocks.  The void cast is used to prevent compiler warnings. */
	pMemPoolConfig->xpStart->pxNextFreeBlock = ( void * ) pucAlignedHeap;
	pMemPoolConfig->xpStart->xBlockSize = ( size_t ) 0;

	/* pxEnd is used to mark the end of the list of free blocks and is inserted
	at the end of the heap space. */
	uxAddress = ( ( size_t ) pucAlignedHeap ) + xTotalHeapSize;
	uxAddress -= pMemPoolConfig->mem_pool_struct_size;
	uxAddress &= ~( ( size_t ) portBYTE_ALIGNMENT_MASK );
	*(pMemPoolConfig->ppxEnd) = ( void * ) uxAddress;
	(*(pMemPoolConfig->ppxEnd))->xBlockSize = 0;
	(*(pMemPoolConfig->ppxEnd))->pxNextFreeBlock = NULL;

	/* To start with there is a single free block that is sized to take up the
	entire heap space, minus the space taken by pxEnd. */
	pxFirstFreeBlock = ( void * ) pucAlignedHeap;
	pxFirstFreeBlock->xBlockSize = uxAddress - ( size_t ) pxFirstFreeBlock;
	pxFirstFreeBlock->pxNextFreeBlock = (*(pMemPoolConfig->ppxEnd));

	/* Only one block exists - and it covers the entire usable heap space. */
	*(pMemPoolConfig->pxMinimumEverFreeBytesRemaining) = pxFirstFreeBlock->xBlockSize;
	*(pMemPoolConfig->pxFreeBytesRemaining) = pxFirstFreeBlock->xBlockSize;

	/* Work out the position of the top bit in a size_t variable. */
	//xBlockAllocatedBit = ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 );
	// replaced by (pMemPoolConfig->mem_pool_blk_allocatedbit)
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList( const MemPoolConfig_t * const pMemPoolConfig, BlockLink_t *pxBlockToInsert )
{
BlockLink_t *pxIterator;
uint8_t *puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
	for( pxIterator = pMemPoolConfig->xpStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock )
	{
		/* Nothing to do here, just iterate to the right position. */
	}

	/* Do the block being inserted, and the block it is being inserted after
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxIterator;
	if( ( puc + pxIterator->xBlockSize ) == ( uint8_t * ) pxBlockToInsert )
	{
		pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
		pxBlockToInsert = pxIterator;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}

	/* Do the block being inserted, and the block it is being inserted before
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxBlockToInsert;
	if( ( puc + pxBlockToInsert->xBlockSize ) == ( uint8_t * ) pxIterator->pxNextFreeBlock )
	{
		if( pxIterator->pxNextFreeBlock != *(pMemPoolConfig->ppxEnd) )
		{
			/* Form one big block from the two blocks. */
			pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
			pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
		}
		else
		{
			pxBlockToInsert->pxNextFreeBlock = *(pMemPoolConfig->ppxEnd);
		}
	}
	else
	{
		pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
	}

	/* If the block being inserted plugged a gab, so was merged with the block
	before and the block after, then it's pxNextFreeBlock pointer will have
	already been set, and should not be set here as that would make it point
	to itself. */
	if( pxIterator != pxBlockToInsert )
	{
		pxIterator->pxNextFreeBlock = pxBlockToInsert;
	}
	else
	{
		mtCOVERAGE_TEST_MARKER();
	}
}


static void prvAddBlockIntoPendingFreeList( const MemPoolConfig_t * const pMemPoolConfig, BlockLink_t *pxBlockToAdd )
{
	SysCriticalLevel_t orig_SysCriticalLevel = arch4rtos_iGetSysCriticalLevel();

	arch4rtos_iRaiseSysCriticalLevel(configMemCriticalLevel);

	pxBlockToAdd->pxNextFreeBlock = *(pMemPoolConfig->ppxPendingFree);
	*(pMemPoolConfig->ppxPendingFree) = pxBlockToAdd;

	//pMemPoolConfig->pxPendingFree->pxNextFreeBlock = pxBlockToAdd->pxNextFreeBlock;

	arch4rtos_iDropSysCriticalLevel(orig_SysCriticalLevel);
}

static BlockLink_t * prvPickBlockFromPendingFreeList	//()
							( const MemPoolConfig_t * const pMemPoolConfig)
{
	SysCriticalLevel_t orig_SysCriticalLevel = arch4rtos_iGetSysCriticalLevel();
	BlockLink_t * pBlockToBeFree;

	arch4rtos_iRaiseSysCriticalLevel(configMemCriticalLevel);  // raise SysCriticalLevel{

	pBlockToBeFree = *(pMemPoolConfig->ppxPendingFree);

	if(pBlockToBeFree != NULL)
	{
		*(pMemPoolConfig->ppxPendingFree) = pBlockToBeFree->pxNextFreeBlock;
	}

	arch4rtos_iDropSysCriticalLevel(orig_SysCriticalLevel);	// restore SysCriticalLevel}
	return pBlockToBeFree;
}
