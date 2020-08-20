#include "os.h"


/* 初始化时基列表的数据域 */
void OS_TickListInit (void)
{
    OS_OBJ_QTY i;
    OS_TICK_SPOKE *p_spoke;
    
    for (i = 0u; i < OSCfg_TickWheelSize; i++)
    {
        p_spoke = (OS_TICK_SPOKE *)&OSCfg_TickWheel[i];
        p_spoke->FirstPtr = (OS_TCB *)0;
        p_spoke->NbrEntries = (OS_OBJ_QTY )0u;
        p_spoke->NbrEntriesMax = (OS_OBJ_QTY )0u;
    }
}




/* 将一个任务插入到时基列表，根据延时时间的大小升序排列 */
void OS_TickListInsert (OS_TCB *p_tcb,OS_TICK time)
{
    OS_OBJ_QTY spoke;
    OS_TICK_SPOKE *p_spoke;
    OS_TCB *p_tcb0;
    OS_TCB *p_tcb1;
    
    p_tcb->TickCtrMatch = OSTickCtr + time;
    p_tcb->TickRemain = time;
    
    spoke = (OS_OBJ_QTY)(p_tcb->TickCtrMatch % OSCfg_TickWheelSize);
    p_spoke = &OSCfg_TickWheel[spoke];
    
    /* 插入到 OSCfg_TickWheel[spoke] 的第一个节点 */
    if (p_spoke->NbrEntries == (OS_OBJ_QTY)0u) 
    {
        p_tcb->TickNextPtr = (OS_TCB *)0;
        p_tcb->TickPrevPtr = (OS_TCB *)0;
        p_spoke->FirstPtr = p_tcb;
        p_spoke->NbrEntries = (OS_OBJ_QTY)1u;
    }
    /* 如果插入的不是第一个节点，则按照TickRemain大小升序排列 */
    else
    {
        /* 获取第一个节点指针 */
        p_tcb1 = p_spoke->FirstPtr;
        while (p_tcb1 != (OS_TCB *)0)
        {
            /* 计算比较节点的剩余时间 */
            p_tcb1->TickRemain = p_tcb1->TickCtrMatch - OSTickCtr;
            
            if (p_tcb->TickRemain > p_tcb1->TickRemain) 
            {
                if (p_tcb1->TickNextPtr != (OS_TCB *)0)
                {
                    p_tcb1 = p_tcb1->TickNextPtr;
                }
                else
                {
                    p_tcb->TickNextPtr = (OS_TCB *)0;
                    p_tcb->TickPrevPtr = p_tcb1;
                    p_tcb1->TickNextPtr = p_tcb;
                    p_tcb1 = (OS_TCB *)0;//退出
                }
            }
            else
            {
                if (p_tcb1->TickPrevPtr == (OS_TCB *)0)
                {
                    p_tcb->TickPrevPtr = (OS_TCB *)0;
                    p_tcb->TickNextPtr = p_tcb1;
                    p_tcb1->TickPrevPtr = p_tcb;
                    p_spoke->FirstPtr = p_tcb;
                }
                else
                {
                    p_tcb0 = p_tcb1->TickPrevPtr;
                    p_tcb->TickPrevPtr = p_tcb0;
                    p_tcb0 ->TickNextPtr = p_tcb;
                    
                    p_tcb1->TickPrevPtr = p_tcb;
                    p_tcb ->TickNextPtr = p_tcb1; 
                }
                p_tcb1 = (OS_TCB *)0;
            }
        }
        /* 节点成功插入 */
        p_spoke->NbrEntries++;
    }
    
    /* 刷新NbrEntriesMax的值 */
    if (p_spoke->NbrEntriesMax < p_spoke->NbrEntries) 
    {
        p_spoke->NbrEntriesMax = p_spoke->NbrEntries;
    }
    
    /* 任务TCB中的TickSpokePtr回指根节点 */
    p_tcb->TickSpokePtr = p_spoke;
    
}


void OS_TickListRemove (OS_TCB *p_tcb)
{
    OS_TICK_SPOKE *p_spoke;
    OS_TCB *p_tcb1;
    OS_TCB *p_tcb2;
    
    /* 获取任务TCB所在链表的根指针 */
    p_spoke = p_tcb->TickSpokePtr;
    
    if (p_spoke != (OS_TICK_SPOKE *)0)
    {
        p_tcb->TickRemain = (OS_TICK)0u;
        
        /* 要移除的刚好是第一个节点 */
        if (p_spoke->FirstPtr == p_tcb)
        {
            p_tcb1 = (OS_TCB *)p_tcb->TickNextPtr;
            p_spoke->FirstPtr = p_tcb1;
            if (p_tcb1 != (OS_TCB *)0)
            {
                p_tcb1->TickPrevPtr = (OS_TCB *)0;
            }
        }
        else
        {
            p_tcb1 = p_tcb->TickPrevPtr;
            p_tcb2 = p_tcb->TickNextPtr;
            
            /* 节点移除，将节点前后的两个节点连接在一起 */
            p_tcb1->TickNextPtr = p_tcb2;
            if (p_tcb2 != (OS_TCB *)0)
            {
                p_tcb2->TickPrevPtr = p_tcb1;
            }
        } 
        p_tcb->TickNextPtr = (OS_TCB *)0;
        p_tcb->TickPrevPtr = (OS_TCB *)0;
        p_tcb->TickSpokePtr = (OS_TICK_SPOKE *)0;
        p_tcb->TickCtrMatch = (OS_TICK )0u;
        
        p_spoke->NbrEntries--;
    }
}

void OS_TaskRdy (OS_TCB *p_tcb)
{
    /* 从时基列表删除 */
    OS_TickListRemove(p_tcb);
    /* 插入就绪列表 */
    OS_RdyListInsert(p_tcb);
}

void OS_TickListUpdate (void)
{
    OS_OBJ_QTY spoke;
    OS_TICK_SPOKE *p_spoke;
    OS_TCB *p_tcb;
    OS_TCB *p_tcb_next;
    OS_TCB       *p_tcb_owner;
    OS_PRIO       prio_new;
    
    CPU_BOOLEAN done;
    
    CPU_SR_ALLOC();
    
    /* 进入临界段 */
    CPU_INT_DIS();
    
    /* 时基计数器++ */
    OSTickCtr++;
    spoke = (OS_OBJ_QTY)(OSTickCtr % OSCfg_TickWheelSize);
    p_spoke = &OSCfg_TickWheel[spoke];
    
    p_tcb = p_spoke->FirstPtr;
    done = 0;
    
    while (done == 0)
    {
        if (p_tcb != (OS_TCB *)0)
        {
            p_tcb_next = p_tcb->TickNextPtr;
            switch (p_tcb->TaskState) 
            {                              
                case OS_TASK_STATE_RDY:                 
                case OS_TASK_STATE_PEND:
                case OS_TASK_STATE_SUSPENDED:
                case OS_TASK_STATE_PEND_SUSPENDED:
                     break;

                case OS_TASK_STATE_DLY:                               //如果是延时状态
                     p_tcb->TickRemain = p_tcb->TickCtrMatch          //计算延时的的剩余时间 
                                       - OSTickCtr;
                     if (OSTickCtr == p_tcb->TickCtrMatch) 
                     {        
                         p_tcb->TaskState = OS_TASK_STATE_RDY;        //修改任务状态量为就绪状态
                         OS_TaskRdy(p_tcb);                           //让任务就绪
                     } 
                     else 
                     {                                   
                         done             = 1;              
                     }
                     break;

                case OS_TASK_STATE_PEND_TIMEOUT:                      //如果是有期限等待状态
                case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:            //如果是有期限等待中被挂起状态
                    
                     p_tcb->TickRemain = p_tcb->TickCtrMatch          //计算期限的的剩余时间
                                       - OSTickCtr;
                     if (OSTickCtr == p_tcb->TickCtrMatch) 
                     {         
                        p_tcb_owner = (OS_TCB *)0;
                        if (p_tcb->PendOn == OS_TASK_PEND_ON_MUTEX) 
                        {
                            p_tcb_owner = (OS_TCB *)((OS_MUTEX *)p_tcb->PendObjPtr)->OwnerTCBPtr;
                        }

                        OS_PendListRemove(p_tcb); 

                        if (p_tcb->TaskState == OS_TASK_STATE_PEND_TIMEOUT) 
                        {
                            OS_TaskRdy(p_tcb);
                            p_tcb->TaskState  = OS_TASK_STATE_RDY;

                        } 
                        else 
                        {
                            if (p_tcb->TaskState == OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED) 
                            {
                                OS_TickListRemove(p_tcb);
                                p_tcb->TaskState  = OS_TASK_STATE_SUSPENDED;
                            }
                        }

                        p_tcb->PendStatus = OS_STATUS_PEND_TIMEOUT;         /* Indicate pend timed out                              */
                        p_tcb->PendOn     = OS_TASK_PEND_ON_NOTHING;        /* Indicate no longer pending                           */

                        if (p_tcb_owner != 0) 
                        {
                            //如果等待互斥量的任务发生超时，那么就不再等待互斥量，刚好这个任务的优先级是最高的，那么
                            //拥有该互斥量的任务和等待该互斥量的任务的优先级也得重新调整
                            if ((p_tcb_owner->Prio != p_tcb_owner->BasePrio) &&(p_tcb_owner->Prio == p_tcb->Prio)) 
                            {    
                                prio_new = OS_MutexGrpPrioFindHighest(p_tcb_owner);
                                prio_new = (prio_new > p_tcb_owner->BasePrio) ? p_tcb_owner->BasePrio : prio_new;
                                if(prio_new != p_tcb_owner->Prio) 
                                {
                                    OS_TaskChangePrio(p_tcb_owner, prio_new);
                                }
                            }
                        }                         
                     } 
                     else 
                     {                                      
                         done              = 1;             
                     }
                     break;

                case OS_TASK_STATE_DLY_SUSPENDED:                     //如果是延时中被挂起状态
                     p_tcb->TickRemain = p_tcb->TickCtrMatch          //计算延时的的剩余时间 
                                       - OSTickCtr;
                     if (OSTickCtr == p_tcb->TickCtrMatch) 
                     {     
                         p_tcb->TaskState  = OS_TASK_STATE_SUSPENDED; //修改任务状态量为被挂起状态
                         OS_TickListRemove(p_tcb);                    //从节拍列表移除该任务
                     } 
                     else 
                     {                                        
                         done              = 1;          
                     }
                     break;
                default:
                     break;
            }
            
            p_tcb = p_tcb_next;
        }
        else
        {
            done = 1;
        }
    }
    
    CPU_INT_EN();/* 退出临界段 */
 
}

