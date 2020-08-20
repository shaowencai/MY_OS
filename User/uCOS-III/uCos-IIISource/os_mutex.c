#include "os.h"


void  OS_MutexClr (OS_MUTEX  *p_mutex)
{
    p_mutex->Type              =  0;            

    p_mutex->NamePtr           = (CPU_CHAR *)((void *)"?MUTEX");

    p_mutex->MutexGrpNextPtr   = 0;
    p_mutex->OwnerTCBPtr       = 0;
    p_mutex->OwnerNestingCtr   = 0u;

    p_mutex->TS                = 0u;

    OS_PendListInit(&p_mutex->PendList);
}


void  OSMutexCreate (OS_MUTEX  *p_mutex,
                     CPU_CHAR  *p_name,
                     OS_ERR    *p_err)
{
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();

    p_mutex->Type              =  0;            

    p_mutex->NamePtr           =  p_name;

    p_mutex->MutexGrpNextPtr   = 0;
    p_mutex->OwnerTCBPtr       = 0;
    p_mutex->OwnerNestingCtr   = 0u;                     
    p_mutex->TS                = 0u;

    OS_PendListInit(&p_mutex->PendList);                        

    CPU_CRITICAL_EXIT();
    *p_err = OS_ERR_NONE;
}

void  OS_MutexGrpRemove (OS_TCB  *p_tcb, OS_MUTEX  *p_mutex)
{
    OS_MUTEX  **pp_mutex;

    pp_mutex = &p_tcb->MutexGrpHeadPtr;

    while(*pp_mutex != p_mutex) 
    {
        pp_mutex = &(*pp_mutex)->MutexGrpNextPtr;
    }

    *pp_mutex = (*pp_mutex)->MutexGrpNextPtr;
}



OS_OBJ_QTY  OSMutexDel (OS_MUTEX  *p_mutex,
                        OS_OPT     opt,
                        OS_ERR    *p_err)
{
    OS_OBJ_QTY     nbr_tasks;
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_tcb;
    OS_TCB        *p_tcb_owner;
    OS_PRIO        prio_new;

    CPU_SR_ALLOC();
    
    CPU_CRITICAL_ENTER();
    p_pend_list = &p_mutex->PendList;
    nbr_tasks   = 0u;
    switch (opt) 
    {
        case OS_OPT_DEL_NO_PEND:
        {            
             if (p_pend_list->HeadPtr == 0) 
             {
                 if (p_mutex->OwnerTCBPtr != 0) 
                 {        
                     OS_MutexGrpRemove(p_mutex->OwnerTCBPtr, p_mutex); /* yes, remove it from the task group.           */
                 }
                 OS_MutexClr(p_mutex);
                 CPU_CRITICAL_EXIT();
                *p_err = OS_ERR_NONE;
             } 
             else 
             {
                 CPU_CRITICAL_EXIT();
                *p_err = OS_ERR_TASK_WAITING;
             }
             break;
        }

        case OS_OPT_DEL_ALWAYS:
        {            
             while (p_pend_list->HeadPtr != 0) 
             {         
                 p_tcb = p_pend_list->HeadPtr;
                 OS_PendAbort(p_tcb,0,OS_STATUS_PEND_DEL);
                 nbr_tasks++;
             }
             p_tcb_owner = p_mutex->OwnerTCBPtr;
             if (p_tcb_owner != 0) {                     /* Does the mutex belong to a task?                     */
                 OS_MutexGrpRemove(p_tcb_owner, p_mutex);       /* yes, remove it from the task group.                  */
             }


             if (p_tcb_owner != 0) 
             {                    
                 if (p_tcb_owner->Prio != p_tcb_owner->BasePrio) 
                 {
                     prio_new = OS_MutexGrpPrioFindHighest(p_tcb_owner);
                     prio_new = (prio_new > p_tcb_owner->BasePrio) ? p_tcb_owner->BasePrio : prio_new;
                     OS_TaskChangePrio(p_tcb_owner, prio_new);
                 }
             }

             OS_MutexClr(p_mutex);
             CPU_CRITICAL_EXIT();
             OSSched();                                         /* Find highest priority task ready to run              */
            *p_err = OS_ERR_NONE;
             break;
         }

        default:
             CPU_CRITICAL_EXIT();
            *p_err = OS_ERR_OPT_INVALID;
             break;
    }
    return (nbr_tasks);
}

void  OS_MutexGrpAdd (OS_TCB  *p_tcb, OS_MUTEX  *p_mutex)
{
    p_mutex->MutexGrpNextPtr = p_tcb->MutexGrpHeadPtr;      /* The mutex grp is not sorted add to head of list.       */
    p_tcb->MutexGrpHeadPtr   = p_mutex;
}


void  OSMutexPend (OS_MUTEX  *p_mutex,
                   OS_TICK    timeout,
                   OS_OPT     opt,
                   CPU_TS    *p_ts,
                   OS_ERR    *p_err)
{
    OS_TCB  *p_tcb;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    if (p_mutex->OwnerNestingCtr == 0u) 
    {                    
        p_mutex->OwnerTCBPtr     = OSTCBCurPtr;                 /* Yes, caller may proceed                              */
        p_mutex->OwnerNestingCtr = 1u;

        OS_MutexGrpAdd(OSTCBCurPtr, p_mutex);                   /* Add mutex to owner's group                           */
        CPU_CRITICAL_EXIT();
       *p_err = OS_ERR_NONE;
        return;
    }

    if (OSTCBCurPtr == p_mutex->OwnerTCBPtr)                    /* See if current task is already the owner of the mutex*/
    {                 
        if (p_mutex->OwnerNestingCtr == (OS_NESTING_CTR)-1) 
        {
            CPU_CRITICAL_EXIT();
           *p_err = OS_ERR_MUTEX_OVF;
            return;
        }
        p_mutex->OwnerNestingCtr++;

        CPU_CRITICAL_EXIT();
        *p_err = OS_ERR_MUTEX_OWNER;                             /* Indicate that current task already owns the mutex    */
        return;
    }

    if ((opt & OS_OPT_PEND_NON_BLOCKING) != 0u) 
    {            
        CPU_CRITICAL_EXIT();

       *p_err = OS_ERR_PEND_WOULD_BLOCK;                    
        return;
    } 

    p_tcb = p_mutex->OwnerTCBPtr;                               /* Point to the TCB of the Mutex owner                  */
    
    if (p_tcb->Prio > OSTCBCurPtr->Prio)                        /* See if mutex owner has a lower priority than current */
    {                     
        OS_TaskChangePrio(p_tcb, OSTCBCurPtr->Prio);
    }

    OS_Pend((OS_PEND_OBJ *)((void *)p_mutex),                   /* Block task pending on Mutex                          */
             OS_TASK_PEND_ON_MUTEX,
             timeout);

    CPU_CRITICAL_EXIT();
    OSSched();                                                  /* Find the next highest priority task ready to run     */

    CPU_CRITICAL_ENTER();
    switch (OSTCBCurPtr->PendStatus) 
    {
        case OS_STATUS_PEND_OK:                                 /* We got the mutex                                     */

            *p_err = OS_ERR_NONE;
             break;

        case OS_STATUS_PEND_ABORT:                              /* Indicate that we aborted                             */

            *p_err = OS_ERR_PEND_ABORT;
             break;

        case OS_STATUS_PEND_TIMEOUT:                            /* Indicate that we didn't get mutex within timeout     */

            *p_err = OS_ERR_TIMEOUT;
             break;

        case OS_STATUS_PEND_DEL:                                /* Indicate that object pended on has been deleted      */

            *p_err = OS_ERR_OBJ_DEL;
             break;

        default:
            *p_err = OS_ERR_STATUS_INVALID;
             break;
    }
    CPU_CRITICAL_EXIT();
}


OS_PRIO  OS_MutexGrpPrioFindHighest (OS_TCB  *p_tcb)
{
    OS_MUTEX  **pp_mutex;
    OS_PRIO     highest_prio;
    OS_PRIO     prio;
    OS_TCB     *p_head;


    highest_prio = (OS_PRIO)(OS_CFG_PRIO_MAX - 1u);
    pp_mutex = &p_tcb->MutexGrpHeadPtr;

    while(*pp_mutex != 0) 
    {
        p_head = (*pp_mutex)->PendList.HeadPtr;
        
        if (p_head != 0) 
        {
            prio = p_head->Prio;
            
            if(prio < highest_prio) 
            {
                highest_prio = prio;
            }
        }
        
        pp_mutex = &(*pp_mutex)->MutexGrpNextPtr;
    }

    return (highest_prio);
}


void  OSMutexPost (OS_MUTEX  *p_mutex,
                   OS_OPT     opt,
                   OS_ERR    *p_err)
{
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_tcb;
    CPU_TS         ts;
    OS_PRIO        prio_new;
    CPU_SR_ALLOC();


    CPU_CRITICAL_ENTER();
    if (OSTCBCurPtr != p_mutex->OwnerTCBPtr) 
    {              
        CPU_CRITICAL_EXIT();
       *p_err = OS_ERR_MUTEX_NOT_OWNER;
        return;
    }

    ts          = 0u;
    p_mutex->OwnerNestingCtr--;                                 /* Decrement owner's nesting counter                    */
    if (p_mutex->OwnerNestingCtr > 0u) 
    {                        
        CPU_CRITICAL_EXIT();                                     /* No                                                   */
       *p_err = OS_ERR_MUTEX_NESTING;
        return;
    }

    OS_MutexGrpRemove(OSTCBCurPtr, p_mutex);                    /* Remove mutex from owner's group                      */

    p_pend_list = &p_mutex->PendList;
    
    if (p_pend_list->HeadPtr == 0) 
    {      
        p_mutex->OwnerTCBPtr     = 0;                   
        p_mutex->OwnerNestingCtr = 0u;
        CPU_CRITICAL_EXIT();
       *p_err = OS_ERR_NONE;
        return;
    }
                                                                /* Yes                                                  */
    if (OSTCBCurPtr->Prio != OSTCBCurPtr->BasePrio) 
    {
        //因为一个TCB可以拥有多个互斥量，当其他任务也想拥有这些互斥量的时候，拥有的和想拥有的优先级要一样
        //这样的话TCB释放一个信号量之后优先级还可能不是自己本身的优先级。
        prio_new = OS_MutexGrpPrioFindHighest(OSTCBCurPtr);     /* Yes, find highest priority pending                   */
        prio_new = (prio_new > OSTCBCurPtr->BasePrio) ? OSTCBCurPtr->BasePrio : prio_new;
        if (prio_new > OSTCBCurPtr->Prio) 
        {
            OS_RdyListRemove(OSTCBCurPtr);
            OSTCBCurPtr->Prio = prio_new;                       /* Lower owner's priority back to its original one      */
            OS_PrioInsert(prio_new);
            OS_RdyListInsertTail(OSTCBCurPtr);                  /* Insert owner in ready list at new priority           */
            OSPrioCur         = prio_new;
        }
    }
                                                                /* Get TCB from head of pend list                       */
    p_tcb                    = p_pend_list->HeadPtr;
    p_mutex->OwnerTCBPtr     = p_tcb;                           /* Give mutex to new owner                              */
    p_mutex->OwnerNestingCtr = 1u;
    OS_MutexGrpAdd(p_tcb, p_mutex);
                                                                /* Post to mutex                                        */
    OS_Post((OS_PEND_OBJ *)p_mutex,
                           p_tcb,
                           0,
                           0u,
                           ts);

    CPU_CRITICAL_EXIT();

    if ((opt & OS_OPT_POST_NO_SCHED) == 0u) 
    {
        OSSched();                                              /* Run the scheduler                                    */
    }
    *p_err = OS_ERR_NONE;
}



