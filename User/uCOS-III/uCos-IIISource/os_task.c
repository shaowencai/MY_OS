#include "os.h"


static void OS_TaskInitTCB (OS_TCB *p_tcb) 
{ 
  p_tcb->StkPtr = (CPU_STK *)0; 
  p_tcb->StkSize = (CPU_STK_SIZE )0u; 
  p_tcb->TaskDelayTicks = (OS_TICK )0u; 
  p_tcb->Prio = (OS_PRIO )32;  
  p_tcb->NextPtr = (OS_TCB *)0; 
  p_tcb->PrevPtr = (OS_TCB *)0;
}


void OSTaskCreate (OS_TCB       *p_tcb,
                   OS_TASK_PTR  p_task,
                   void         *p_arg,
                   OS_PRIO      prio,
                   CPU_STK      *p_stk_base,
                   CPU_STK_SIZE stk_size,
                   OS_TICK      time_quanta,
                   OS_ERR       *p_err)
{
    CPU_STK *p_sp;
    CPU_SR_ALLOC();


    CPU_INT_DIS();
    
    /* 初始化TCB为默认值 */
    OS_TaskInitTCB(p_tcb);

    p_sp = OSTaskStkInit (p_task,
                        p_arg,
                        p_stk_base,
                        stk_size);
    p_tcb->Prio = prio;
    p_tcb->StkPtr = p_sp;
    p_tcb->StkSize = stk_size;

    /* 时间片相关初始化 */
    p_tcb->TimeQuanta = time_quanta;//表示任务能享有的最大的时间片是多少,该值一旦初始化后就不会变
#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u
    p_tcb->TimeQuantaCtr = time_quanta;//每经过一个系统时钟周期，该值会递减，如果该值为0，则表示时间片耗完
#endif

    OS_PrioInsert(p_tcb->Prio); 
    OS_RdyListInsertTail(p_tcb);
    CPU_INT_EN();

    *p_err = OS_ERR_NONE;
}




#if OS_CFG_TASK_SUSPEND_EN > 0u
void OSTaskSuspend (OS_TCB *p_tcb,
                    OS_ERR *p_err)
{
    
    CPU_SR_ALLOC();

    
#if OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u    
    /* 不能在ISR程序中调用该函数 */
    if (OSIntNestingCtr > (OS_NESTING_CTR)0)
    {
        *p_err = OS_ERR_TASK_SUSPEND_ISR;
        return;
    }
#endif
    
    /* 不能挂起空闲任务 */
    if (p_tcb == &OSIdleTaskTCB)
    {
        *p_err = OS_ERR_TASK_SUSPEND_IDLE;
        return;
    }
    
    CPU_INT_DIS();
    
    if (p_tcb == (OS_TCB *)0)
    {
        if (OSRunning != OS_STATE_OS_RUNNING)
        {                
            /* Can't suspend self when the kernel isn't running     */
            CPU_INT_EN();
            *p_err = OS_ERR_OS_NOT_RUNNING;
            return;
        }
        p_tcb = OSTCBCurPtr;
    }
    if (p_tcb == OSTCBCurPtr)
    {
        /* 如果调度器锁住则不能挂起自己 */
        if (OSSchedLockNestingCtr > (OS_NESTING_CTR)0)
        {
            CPU_INT_EN();
            *p_err = OS_ERR_SCHED_LOCKED;
            return;
        }
    }
    
    *p_err = OS_ERR_NONE;
    switch (p_tcb->TaskState)
    {
        case OS_TASK_STATE_RDY:
            p_tcb->TaskState  =  OS_TASK_STATE_SUSPENDED;
            p_tcb->SuspendCtr = 1u;
            OS_RdyListRemove(p_tcb);
            CPU_INT_EN();
            break;
        case OS_TASK_STATE_DLY:
            p_tcb->TaskState  = OS_TASK_STATE_DLY_SUSPENDED;
            p_tcb->SuspendCtr = 1u;    
            CPU_INT_EN();   
            break;      
        case OS_TASK_STATE_PEND:
            p_tcb->TaskState  = OS_TASK_STATE_PEND_SUSPENDED;
            p_tcb->SuspendCtr = 1u;
            CPU_INT_EN();
            break;       
        case OS_TASK_STATE_PEND_TIMEOUT:
            p_tcb->TaskState  = OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED;
            p_tcb->SuspendCtr = 1u;
            CPU_INT_EN();
             break;
        case OS_TASK_STATE_SUSPENDED:
        case OS_TASK_STATE_DLY_SUSPENDED:
        case OS_TASK_STATE_PEND_SUSPENDED:
        case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:
            p_tcb->SuspendCtr++;
            CPU_INT_EN();
            break;
        default:
            CPU_INT_EN();
            *p_err = OS_ERR_STATE_INVALID;
            return;
    }
    /* 任务切换 */
    OSSched();  
}
#endif


#if OS_CFG_TASK_SUSPEND_EN > 0u
void OSTaskResume (OS_TCB *p_tcb,
                    OS_ERR *p_err)
{
    CPU_SR_ALLOC();
    
#if OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u    
    /* 不能在ISR程序中调用该函数 */
    if (OSIntNestingCtr > (OS_NESTING_CTR)0)
    {
        *p_err = OS_ERR_TASK_RESUME_ISR;
        return;
    }
#endif
    
    CPU_INT_DIS();
    *p_err = OS_ERR_NONE;
    
    switch (p_tcb->TaskState)
    {
        case OS_TASK_STATE_RDY:
        case OS_TASK_STATE_DLY:     
        case OS_TASK_STATE_PEND:      
        case OS_TASK_STATE_PEND_TIMEOUT:
            CPU_INT_EN();
            *p_err = OS_ERR_TASK_NOT_SUSPENDED;
            break; 
        
        case OS_TASK_STATE_SUSPENDED:
            p_tcb->SuspendCtr--;
            if (p_tcb->SuspendCtr == (OS_NESTING_CTR)0)
            {
                p_tcb->TaskState = OS_TASK_STATE_RDY;
                OS_TaskRdy(p_tcb);
            }
            CPU_INT_EN();
            break;
        case OS_TASK_STATE_DLY_SUSPENDED:
             p_tcb->SuspendCtr--;
             if (p_tcb->SuspendCtr == 0u) 
             {
                 p_tcb->TaskState = OS_TASK_STATE_DLY;
             }
             CPU_INT_EN();
             break;
        case OS_TASK_STATE_PEND_SUSPENDED:
             p_tcb->SuspendCtr--;
             if (p_tcb->SuspendCtr == 0u) 
             {
                 p_tcb->TaskState = OS_TASK_STATE_PEND;
             }      
             CPU_INT_EN();
             break;             
        case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:
             p_tcb->SuspendCtr--;
             if (p_tcb->SuspendCtr == 0u) 
             {
                 p_tcb->TaskState = OS_TASK_STATE_PEND_TIMEOUT;
             }
            CPU_INT_EN();
            break;
        default:
            CPU_INT_EN();
            *p_err = OS_ERR_STATE_INVALID;
            return;
    }
    
    /* 任务切换 */
    OSSched(); 
}
#endif



void  OS_TaskChangePrio(OS_TCB  *p_tcb, OS_PRIO  prio_new)
{
    OS_TCB  *p_tcb_owner;
    OS_PRIO  prio_cur;
    do 
    {
        p_tcb_owner = 0;
        prio_cur    = p_tcb->Prio;
        switch (p_tcb->TaskState) 
        {
            case OS_TASK_STATE_RDY:
                 OS_RdyListRemove(p_tcb);                       /* Remove from current priority                         */
                 p_tcb->Prio = prio_new;                        /* Set new task priority                                */
                 OS_PrioInsert(p_tcb->Prio);
                 if (p_tcb == OSTCBCurPtr) 
                 {
                     OS_RdyListInsertHead(p_tcb);
                 } 
                 else 
                 {
                     OS_RdyListInsertTail(p_tcb);
                 }
                 break;

            case OS_TASK_STATE_DLY:                             /* Nothing to do except change the priority in the OS_TCB*/
            case OS_TASK_STATE_SUSPENDED:
            case OS_TASK_STATE_DLY_SUSPENDED:
                 p_tcb->Prio = prio_new;                        /* Set new task priority                                */
                 break;

            case OS_TASK_STATE_PEND:
            case OS_TASK_STATE_PEND_TIMEOUT:
            case OS_TASK_STATE_PEND_SUSPENDED:
            case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:
                 p_tcb->Prio = prio_new;                        /* Set new task priority                                */
                 switch (p_tcb->PendOn) 
                 {                       
                     case OS_TASK_PEND_ON_FLAG:
                     case OS_TASK_PEND_ON_Q:
                     case OS_TASK_PEND_ON_SEM:
                          OS_PendListChangePrio(p_tcb);
                          break;

                     case OS_TASK_PEND_ON_MUTEX:
                          OS_PendListChangePrio(p_tcb);
                          p_tcb_owner = ((OS_MUTEX *)p_tcb->PendObjPtr)->OwnerTCBPtr;
                          if (prio_cur > prio_new) 
                          {          
                              if (p_tcb_owner->Prio <= prio_new) 
                              {
                                  p_tcb_owner = 0;
                              } 

                          }
                          else //这个else分支理解不了啊
                          {
                              if (p_tcb_owner->Prio == prio_cur)
                              { 
                                  prio_new = OS_MutexGrpPrioFindHighest(p_tcb_owner);
                                  prio_new = (prio_new > p_tcb_owner->BasePrio) ? p_tcb_owner->BasePrio : prio_new;
                                  if (prio_new == p_tcb_owner->Prio) 
                                  {
                                      p_tcb_owner = 0;
                                  } 
                              }
                          }
                          break;

                     case OS_TASK_PEND_ON_TASK_Q:
                     case OS_TASK_PEND_ON_TASK_SEM:
                     default:
                          break;
                 }
                 break;

            default:
                 return;
        }
        p_tcb = p_tcb_owner;
    } while (p_tcb != 0);
}


