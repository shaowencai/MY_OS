#include "lib_std.h"
#include "heap.h"


void My_Mem_Init(void)
{
    system_heap_init(MyHeap_SRAM_BEGIN,MyHeap_SRAM_END);
}

void * My_malloc( CPU_INT32U length  )
{
    void *pv;	
    CPU_SR_ALLOC();  
    CPU_CRITICAL_ENTER();
    pv = pvPortMalloc(length);
    CPU_CRITICAL_EXIT();
    return pv;
}

void My_free( void *pv )
{
    CPU_SR_ALLOC();  
    CPU_CRITICAL_ENTER();
    vPortFree(pv);
    CPU_CRITICAL_EXIT();
    return;
}


void *My_realloc(void *rmem, CPU_INT32U newsize)
{
    void *pv;
    CPU_SR_ALLOC();  
    CPU_CRITICAL_ENTER();
    pv = vPortRealloc(rmem,newsize);
    CPU_CRITICAL_EXIT();
    return pv;
}
