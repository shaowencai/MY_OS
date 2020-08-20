#define _USER_HEAP_INNERDEF_
#include "heap.h"


static CPU_INT32U HeapStartAddr;
static CPU_INT32U TOTAL_HEAP_SIZE;

typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	CPU_INT32U xBlockSize;					/*<< The size of the free block. */
} BlockLink_t;


/***************************************************************************
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 ***************************************************************************/
static void prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert );

/***************************************************************************
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 ***************************************************************************/
static void prvHeapInit( void );

/*-------------------------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const CPU_INT32U xHeapStructSize = ( sizeof( BlockLink_t ) + ( ( size_t ) ( portBYTE_ALIGNMENT - 1 ) ) )\
										   & ~( ( CPU_INT32U ) portBYTE_ALIGNMENT_MASK ); //8字节对齐

/* Create a couple of list links to mark the start and end of the list. */
static BlockLink_t xStart, *pxEnd = NULL;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static CPU_INT32U xFreeBytesRemaining = 0U;//目前堆剩余多少字节
static CPU_INT32U xMinimumEverFreeBytesRemaining = 0U;//在堆的使用历史中剩余最少时的字节数

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
member of an BlockLink_t structure is set then the block belongs to the
application.  When the bit is free the block is still part of the free heap
space. */
static CPU_INT32U xBlockAllocatedBit = 0;

/*-----------------------------------------------------------*/
void system_heap_init(CPU_INT32U SA,CPU_INT32U EA)
{   
	 HeapStartAddr = SA;
	 TOTAL_HEAP_SIZE = (EA-SA); 
}

/*-----------------------------------------------------------*/

void Mem_Copy(void *dst, const void *src, CPU_INT32U count)
{
    #ifdef RT_TINY_SIZE
    char *tmp = (char *)dst, *s = (char *)src;

    while (count--)
        *tmp++ = *s++;

    return dst;
    #else

    #define UNALIGNED(X, Y)  (((CPU_INT32U)X & (sizeof(CPU_INT32U) - 1)) | \
                             ((CPU_INT32U)Y & (sizeof(CPU_INT32U) - 1)))
    #define BIGBLOCKSIZE    (sizeof(CPU_INT32U) << 2)
    #define LITTLEBLOCKSIZE (sizeof(CPU_INT32U))
    #define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

    char *dst_ptr = (char *)dst;
    char *src_ptr = (char *)src;
    CPU_INT32U *aligned_dst;
    CPU_INT32U *aligned_src;
    int len = count;

    /* If the size is small, or either SRC or DST is unaligned,
    then punt into the byte copy loop.  This should be rare. */
    if (!TOO_SMALL(len) && !UNALIGNED(src_ptr, dst_ptr))
    {
        aligned_dst = (CPU_INT32U *)dst_ptr;
        aligned_src = (CPU_INT32U *)src_ptr;

        /* Copy 4X long words at a time if possible. */
        while (len >= BIGBLOCKSIZE)
        {
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            len -= BIGBLOCKSIZE;
        }

        /* Copy one long word at a time if possible. */
        while (len >= LITTLEBLOCKSIZE)
        {
            *aligned_dst++ = *aligned_src++;
            len -= LITTLEBLOCKSIZE;
        }

        /* Pick up any residual with a byte copier. */
        dst_ptr = (char *)aligned_dst;
        src_ptr = (char *)aligned_src;
    }

    while (len--)
        *dst_ptr++ = *src_ptr++;

    #undef UNALIGNED
    #undef BIGBLOCKSIZE
    #undef LITTLEBLOCKSIZE
    #undef TOO_SMALL
    #endif
}


void *pvPortMalloc( CPU_INT32U xWantedSize )
{
	BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
	void *pvReturn = NULL;

	/* If this is the first call to malloc then the heap will require
	initialisation to setup the list of free blocks. */
	if( pxEnd == NULL )
	{
		prvHeapInit();
	}

	/* Check the requested block size is not so large that the top bit is
	set.  The top bit of the block size member of the BlockLink_t structure
	is used to determine who owns the block - the application or the
	kernel, so it must be free. */
	if( ( xWantedSize & xBlockAllocatedBit ) == 0 )
	{
		/* The wanted size is increased so it can contain a BlockLink_t
		structure in addition to the requested amount of bytes. */
		if( xWantedSize > 0 )
		{
			xWantedSize += xHeapStructSize;

			/* Ensure that blocks are always aligned to the required number
			of bytes. */
			if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
			{
				/* Byte alignment required. */
				xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
			}
		}

		if( ( xWantedSize > 0 ) && ( xWantedSize <= xFreeBytesRemaining ) )
		{
			/* Traverse the list from the start	(lowest address) block until
			one	of adequate size is found. */
			pxPreviousBlock = &xStart;
			pxBlock = xStart.pxNextFreeBlock;
			while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
			{
				pxPreviousBlock = pxBlock;
				pxBlock = pxBlock->pxNextFreeBlock;
			}

			if( pxBlock != pxEnd )
			{
				/* Return the memory space pointed to - jumping over the
				BlockLink_t structure at its start. */
				pvReturn = ( void * ) ( ( ( CPU_INT08U * ) pxPreviousBlock->pxNextFreeBlock ) + xHeapStructSize );

				/* This block is being returned for use so must be taken out
				of the list of free blocks. */
				pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

				/* If the block is larger than required it can be split into
				two. */
				if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
				{
					/* This block is to be split into two.  Create a new
					block following the number of bytes requested. The void
					cast is used to prevent byte alignment warnings from the
					compiler. */
					pxNewBlockLink = ( void * ) ( ( ( CPU_INT08U * ) pxBlock ) + xWantedSize );

					/* Calculate the sizes of two blocks split from the
					single block. */
					pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
					pxBlock->xBlockSize = xWantedSize;

					/* Insert the new block into the list of free blocks. */
					prvInsertBlockIntoFreeList( pxNewBlockLink );
				}

				xFreeBytesRemaining -= pxBlock->xBlockSize;

				if( xFreeBytesRemaining < xMinimumEverFreeBytesRemaining )
				{
					xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
				}

				/* The block is being returned - it is allocated and owned
				by the application and has no "next" block. */
				pxBlock->xBlockSize |= xBlockAllocatedBit;
				pxBlock->pxNextFreeBlock = NULL;
			}
		}
	}
	return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree( void *pv )
{
	CPU_INT08U *puc = ( CPU_INT08U * ) pv;
	BlockLink_t *pxLink;

	if( pv != NULL )
	{
		/* The memory being freed will have an BlockLink_t structure immediately
		before it. */
		puc -= xHeapStructSize;

		/* This casting is to keep the compiler from issuing warnings. */
		pxLink = ( void * ) puc;


		if( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 )
		{
			if( pxLink->pxNextFreeBlock == NULL )
			{
				/* The block is being returned to the heap - it is no longer
				allocated. */
				pxLink->xBlockSize &= ~xBlockAllocatedBit;

				{
					/* Add this block to the list of free blocks. */
					xFreeBytesRemaining += pxLink->xBlockSize;
					prvInsertBlockIntoFreeList( ( ( BlockLink_t * ) pxLink ) );
				}

			}
		}
	}
}
/*-----------------------------------------------------------*/

void * vPortRealloc( void *pv , CPU_INT32U xWantedSize )
{
	CPU_INT08U *puc =    ( CPU_INT08U * ) pv;
	CPU_INT08U *pvtmp;
	CPU_INT32U  backupsize;	
	BlockLink_t *pxLink;
	void* pvrtn=NULL;
	
	if( pv != NULL )
	{
	
		/* The memory being freed will have an BlockLink_t structure immediately
		before it. */
		puc -= xHeapStructSize;

		/* This casting is to keep the compiler from issuing warnings. */
		pxLink = ( void * ) puc;

		if( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 )
		{
			if( pxLink->pxNextFreeBlock == NULL )
			{
				/* The block is being returned to the heap - it is no longer
				allocated. */
				pxLink->xBlockSize &= ~xBlockAllocatedBit;

				//calculate new blocksize 
				
				if( ( xWantedSize & xBlockAllocatedBit ) == 0 )
		    	{
					/* The wanted size is increased so it can contain a BlockLink_t
					structure in addition to the requested amount of bytes. */
					if( xWantedSize > 0 )
					{
						xWantedSize += xHeapStructSize;

						/* Ensure that blocks are always aligned to the required number
						of bytes. */
						if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
						{
							/* Byte alignment required. */
							xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
							
							if( xWantedSize > pxLink->xBlockSize )
							{
								//need to allocate new memory and copy data
							   	//alloc new space 
								pvtmp = (CPU_INT08U*) pvPortMalloc(xWantedSize);
							   	if (pvtmp)
								{
									//copy old data to new space;
									backupsize = pxLink->xBlockSize-xHeapStructSize;
									Mem_Copy( pvtmp , pv , backupsize );
									//free old memory
									pxLink->xBlockSize|= xBlockAllocatedBit;
									vPortFree((void *)pv);	
									pvrtn = (void *)pvtmp;
								}					
							}
							else
							{
								//use old memory ,cos size is enough
								 pvrtn =  (void *)pv;
							}
						}
			   		}
			 
			 	}
			}
		}
	}
	
	return pvrtn;
}
/*-----------------------------------------------------------*/

CPU_INT32U xPortGetFreeHeapSize( void )
{
	return xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

CPU_INT32U xPortGetMinimumEverFreeHeapSize( void )
{
	return xMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

static void prvHeapInit( void )
{
	BlockLink_t *pxFirstFreeBlock;
	CPU_INT08U *pucAlignedHeap;
	CPU_INT32U uxAddress;
	CPU_INT32U xTotalHeapSize = TOTAL_HEAP_SIZE;

	/* Ensure the heap starts on a correctly aligned boundary. */
	uxAddress = ( CPU_INT32U ) HeapStartAddr;//ucHeap;

	//将堆内存首地址对齐，然后计算内存大小
	if( ( uxAddress & portBYTE_ALIGNMENT_MASK ) != 0 )
	{
		uxAddress += ( portBYTE_ALIGNMENT - 1 );
		uxAddress &= ~( ( CPU_INT32U ) portBYTE_ALIGNMENT_MASK );
		xTotalHeapSize -= uxAddress - ( CPU_INT32U ) HeapStartAddr;
	}

	pucAlignedHeap = ( CPU_INT08U * ) uxAddress;

	/* xStart is used to hold a pointer to the first item in the list of free
	blocks.  The void cast is used to prevent compiler warnings. */
	xStart.pxNextFreeBlock = ( void * ) pucAlignedHeap;
	xStart.xBlockSize = ( CPU_INT32U ) 0;

	/* pxEnd is used to mark the end of the list of free blocks and is inserted
	at the end of the heap space. */
	uxAddress = ( ( CPU_INT32U ) pucAlignedHeap ) + xTotalHeapSize;
	uxAddress -= xHeapStructSize;
	uxAddress &= ~( ( CPU_INT32U ) portBYTE_ALIGNMENT_MASK );
	pxEnd = ( void * ) uxAddress;
	pxEnd->xBlockSize = 0;
	pxEnd->pxNextFreeBlock = NULL;

	/* To start with there is a single free block that is 
	sized to take up the
	entire heap space, minus the space taken by pxEnd. */
	pxFirstFreeBlock = ( void * ) pucAlignedHeap;
	pxFirstFreeBlock->xBlockSize = uxAddress - ( CPU_INT32U ) pxFirstFreeBlock;
	pxFirstFreeBlock->pxNextFreeBlock = pxEnd;

	/* Only one block exists - and it covers the entire usable heap space. */
	xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
	xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;

	/* Work out the position of the top bit in a CPU_INT32U variable. */
	xBlockAllocatedBit = ( ( CPU_INT32U ) 1 ) << ( ( sizeof( CPU_INT32U ) * heapBITS_PER_BYTE ) - 1 );
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList( BlockLink_t *pxBlockToInsert )
{
	BlockLink_t *pxIterator;
	CPU_INT08U *puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
	for( pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock );

	/* Do the block being inserted, and the block it is being inserted after
	make a contiguous block of memory? */
	puc = ( CPU_INT08U * ) pxIterator;
	if( ( puc + pxIterator->xBlockSize ) == ( CPU_INT08U * ) pxBlockToInsert )
	{
		pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
		pxBlockToInsert = pxIterator;
	}

	/* Do the block being inserted, and the block it is being inserted before
	make a contiguous block of memory? */
	puc = ( CPU_INT08U * ) pxBlockToInsert;
	if( ( puc + pxBlockToInsert->xBlockSize ) == ( CPU_INT08U * ) pxIterator->pxNextFreeBlock )
	{
		if( pxIterator->pxNextFreeBlock != pxEnd )
		{
			/* Form one big block from the two blocks. */
			pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
			pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
		}
		else
		{
			pxBlockToInsert->pxNextFreeBlock = pxEnd;
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
}
