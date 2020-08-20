#ifndef  _USER_HEAP_
#define  _USER_HEAP_

#include "cpu.h"

#ifdef _USER_HEAP_INNERDEF_

#define portBYTE_ALIGNMENT					8 //8字节对齐
#define portBYTE_ALIGNMENT_MASK 			( 0x0007 )

#define heapMINIMUM_BLOCK_SIZE				( ( CPU_INT32U ) ( xHeapStructSize << 1 ) )

#define heapBITS_PER_BYTE					( ( CPU_INT32U ) 8 )

#endif

typedef unsigned int                    size_t;
#define NULL                                 0 

void *pvPortMalloc( CPU_INT32U xWantedSize );
void vPortFree( void *pv );	
void *vPortRealloc( void *pv , CPU_INT32U xWantedSize );
void system_heap_init(CPU_INT32U SA,CPU_INT32U EA);
	
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define MyHeap_SRAM_BEGIN    ((CPU_INT32U)(&Image$$RW_IRAM1$$ZI$$Limit))
#define MyHeap_SRAM_END		  (0x20020000)
		
#endif
