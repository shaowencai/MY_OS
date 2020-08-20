#include "os.h"

extern void OS_TmrTask(void);

void OSTimeTick (void)
{
#if 0
    if( OSRdyList[0].HeadPtr->TaskDelayTicks)
    {
        OSRdyList[0].HeadPtr->TaskDelayTicks--;
    }
    if( OSRdyList[1].HeadPtr->TaskDelayTicks)
    {
        OSRdyList[1].HeadPtr->TaskDelayTicks--;
    }
#endif
    
#if 0    
    CPU_DATA i;
    CPU_SR_ALLOC();
    CPU_INT_DIS();
    for (i=0; i<OS_CFG_PRIO_MAX; i++) 
    { 
        if (OSRdyList[i].HeadPtr->TaskDelayTicks > 0) 
        {
            OSRdyList[i].HeadPtr->TaskDelayTicks --; 
            if (OSRdyList[i].HeadPtr->TaskDelayTicks == 0) 
            { 
                /* 为0则表示延时时间到，让任务就绪 */ 
                OS_PrioInsert(i);
            } 
        } 
    }
    CPU_INT_EN();
#endif
    
    OS_TickListUpdate();
    
#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u    
    OS_SchedRoundRobin(&OSRdyList[OSPrioCur]);
#endif
    
    OSTmrUpdateCtr--;                                
    if (OSTmrUpdateCtr == (OS_CTR)0u) 
    {              
        OSTmrUpdateCtr = OSTmrUpdateCnt;           
        OS_TmrTask();
    }
   
    OSSched();
}


void OSTimeDly(OS_TICK dly)
{ 
    CPU_SR_ALLOC();

    CPU_INT_DIS();
    
#if 0
    OSTCBCurPtr->TaskDelayTicks = dly;
    OS_PrioRemove(OSTCBCurPtr->Prio);
#endif
    
    OSTCBCurPtr->TaskState = OS_TASK_STATE_DLY;
    /* 插入到时基列表 */
    OS_TickListInsert(OSTCBCurPtr, dly);
    
    /* 从就绪列表移除 */
    OS_RdyListRemove(OSTCBCurPtr);
    
    CPU_INT_EN();

    OSSched();
}

