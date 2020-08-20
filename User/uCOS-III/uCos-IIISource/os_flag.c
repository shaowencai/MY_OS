#include "os.h"

void  OSFlagCreate (OS_FLAG_GRP  *p_grp,
                    CPU_CHAR     *p_name,
                    OS_FLAGS      flags,
                    OS_ERR       *p_err)
{
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    
    p_grp->Type    = 0;                          /* Set to event flag group type                         */

    p_grp->NamePtr = p_name;

    p_grp->Flags   = flags;                                     /* Set to desired initial value                         */

    OS_PendListInit(&p_grp->PendList);

    CPU_CRITICAL_EXIT();
    
   *p_err = OS_ERR_NONE;
}

void  OS_FlagClr (OS_FLAG_GRP  *p_grp)
{
    OS_PEND_LIST  *p_pend_list;

    p_grp->Type             = 0;

    p_grp->NamePtr          = (CPU_CHAR *)((void *)"?FLAG");

    p_grp->Flags            =  0u;
    
    p_pend_list             = &p_grp->PendList;
    
    OS_PendListInit(p_pend_list);
}




OS_OBJ_QTY  OSFlagDel (OS_FLAG_GRP  *p_grp,
                       OS_OPT        opt,
                       OS_ERR       *p_err)
{
    OS_OBJ_QTY     nbr_tasks;
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_tcb;
    CPU_TS         ts;
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    p_pend_list = &p_grp->PendList;
    nbr_tasks   = 0u;
    switch (opt) 
    {
        case OS_OPT_DEL_NO_PEND:
             if (p_pend_list->HeadPtr == 0) 
             {
                 OS_FlagClr(p_grp);

                 CPU_CRITICAL_EXIT();

                *p_err = OS_ERR_NONE;
             } 
             else 
             {
                 CPU_CRITICAL_EXIT();
                *p_err = OS_ERR_TASK_WAITING;
             }
             break;

        case OS_OPT_DEL_ALWAYS:                                 /* Always delete the event flag group                   */
             ts = 0u;

             while (p_pend_list->HeadPtr != 0) 
             {       
                 p_tcb = p_pend_list->HeadPtr;
                 OS_PendAbort(p_tcb,ts,OS_STATUS_PEND_DEL);
                 nbr_tasks++;
             }

             OS_FlagClr(p_grp);
             CPU_CRITICAL_EXIT();

             OSSched();                                         /* Find highest priority task ready to run              */
            *p_err = OS_ERR_NONE;
             break;

        default:
             CPU_CRITICAL_EXIT();
            *p_err = OS_ERR_OPT_INVALID;
             break;
    }

    return (nbr_tasks);
}


void  OS_FlagBlock (OS_FLAG_GRP  *p_grp,
                    OS_FLAGS      flags,
                    OS_OPT        opt,
                    OS_TICK       timeout)
{
    OSTCBCurPtr->FlagsPend = flags;                             /* Save the flags that we need to wait for              */
    OSTCBCurPtr->FlagsOpt  = opt;                               /* Save the type of wait we are doing                   */
    OSTCBCurPtr->FlagsRdy  = 0u;

    OS_Pend((OS_PEND_OBJ *)((void *)p_grp),OS_TASK_PEND_ON_FLAG,timeout);
}



OS_FLAGS  OSFlagPend (OS_FLAG_GRP  *p_grp,
                      OS_FLAGS      flags,
                      OS_TICK       timeout,
                      OS_OPT        opt,
                      CPU_TS       *p_ts,
                      OS_ERR       *p_err)
{
    CPU_BOOLEAN  consume;
    OS_FLAGS     flags_rdy;
    OS_OPT       mode;
    CPU_SR_ALLOC();

    if ((opt & OS_OPT_PEND_FLAG_CONSUME) != 0u)                 /* See if we need to consume the flags                  */
    {            
        consume = 1;
    } 
    else 
    {
        consume = 0;
    }

    if (p_ts != 0) 
    {
       *p_ts = 0u;                                              /* Initialize the returned timestamp                    */
    }

    mode = opt & OS_OPT_PEND_FLAG_MASK;
    
    CPU_CRITICAL_ENTER();
    switch (mode) 
    {
        case OS_OPT_PEND_FLAG_SET_ALL:                          /* See if all required flags are set                    */
             flags_rdy = (p_grp->Flags & flags);                /* Extract only the bits we want                        */
             if (flags_rdy == flags) 
             {                                                  /* Must match ALL the bits that we want                 */
                 if (consume == 1) 
                 {                                              /* See if we need to consume the flags                  */
                     p_grp->Flags &= ~flags_rdy;                /* Clear ONLY the flags that we wanted                  */
                 }
                 
                 OSTCBCurPtr->FlagsRdy = flags_rdy;             /* Save flags that were ready                           */

                 CPU_CRITICAL_EXIT();                           /* Yes, condition met, return to caller                 */
                *p_err = OS_ERR_NONE;
                 return (flags_rdy);
             } 
             else 
             {                                          
                 if ((opt & OS_OPT_PEND_NON_BLOCKING) != 0u) 
                 {
                     CPU_CRITICAL_EXIT();
                    *p_err = OS_ERR_PEND_WOULD_BLOCK;           /* Specified non-blocking so task would block           */
                     return ((OS_FLAGS)0);
                 } 
                 else 
                 {                                      
                     if (OSSchedLockNestingCtr > 0u)            /* See if called with scheduler locked ...        */
                     {        
                         CPU_CRITICAL_EXIT();
                        *p_err = OS_ERR_SCHED_LOCKED;           /* ... can't PEND when locked                           */
                         return (0u);
                     }
                 }
                                                                /* Lock the scheduler/re-enable interrupts              */
                 OS_FlagBlock(p_grp,
                              flags,
                              opt,
                              timeout);
                 CPU_CRITICAL_EXIT();
             }
             break;

        case OS_OPT_PEND_FLAG_SET_ANY:
             flags_rdy = (p_grp->Flags & flags);                /* Extract only the bits we want                        */
             if (flags_rdy != 0u) 
             {                                                  /* See if any flag set                                  */
                 if (consume == 1) 
                 {                                              /* See if we need to consume the flags                  */
                     p_grp->Flags &= ~flags_rdy;                /* Clear ONLY the flags that we got                     */
                 }
                 OSTCBCurPtr->FlagsRdy = flags_rdy;             /* Save flags that were ready                           */
                 
                 CPU_CRITICAL_EXIT();                           /* Yes, condition met, return to caller                 */

                *p_err = OS_ERR_NONE;
                 return (flags_rdy);
             } 
             else 
             {                                                  /* Block task until events occur or timeout             */
                 if ((opt & OS_OPT_PEND_NON_BLOCKING) != 0u) 
                 {
                     CPU_CRITICAL_EXIT();
                    *p_err = OS_ERR_PEND_WOULD_BLOCK;           /* Specified non-blocking so task would block           */
                     return ((OS_FLAGS)0);
                 } 
                 else 
                 {                                              /* Specified blocking so check is scheduler is locked   */
                     if (OSSchedLockNestingCtr > 0u) 
                     {                                          /* See if called with scheduler locked ...        */
                         CPU_CRITICAL_EXIT();
                        *p_err = OS_ERR_SCHED_LOCKED;           /* ... can't PEND when locked                           */
                         return ((OS_FLAGS)0);
                     }
                 }

                 OS_FlagBlock(p_grp,
                              flags,
                              opt,
                              timeout);
                 CPU_CRITICAL_EXIT();
             }
             break;
        case OS_OPT_PEND_FLAG_CLR_ALL:                          /* See if all required flags are cleared                */
             flags_rdy = (OS_FLAGS)(~p_grp->Flags & flags);     /* Extract only the bits we want                        */
             if (flags_rdy == flags) 
             {                                                  /* Must match ALL the bits that we want                 */
                 if (consume == 1) 
                 {                                              /* See if we need to consume the flags                  */
                     p_grp->Flags |= flags_rdy;                 /* Set ONLY the flags that we wanted                    */
                 }
                 OSTCBCurPtr->FlagsRdy = flags_rdy;             /* Save flags that were ready                           */

                 CPU_CRITICAL_EXIT();                           /* Yes, condition met, return to caller                 */
                *p_err = OS_ERR_NONE;
                 return (flags_rdy);
             } 
             else 
             {                                                  /* Block task until events occur or timeout             */
                 if ((opt & OS_OPT_PEND_NON_BLOCKING) != 0u) 
                 {
                     CPU_CRITICAL_EXIT();
                    *p_err = OS_ERR_PEND_WOULD_BLOCK;           /* Specified non-blocking so task would block           */
                     return ((OS_FLAGS)0);
                 } 
                 else 
                 {                                              /* Specified blocking so check is scheduler is locked   */
                     if (OSSchedLockNestingCtr > 0u) 
                     {                                          /* See if called with scheduler locked ...        */
                         CPU_CRITICAL_EXIT();
                         *p_err = OS_ERR_SCHED_LOCKED;          /* ... can't PEND when locked                           */
                         return (0);
                     }
                 }

                 OS_FlagBlock(p_grp,
                              flags,
                              opt,
                              timeout);
                 CPU_CRITICAL_EXIT();
             }
             break;

        case OS_OPT_PEND_FLAG_CLR_ANY:
             flags_rdy = (~p_grp->Flags & flags);               /* Extract only the bits we want                        */
             if (flags_rdy != 0u) 
             {                                                  /* See if any flag cleared                              */
                 if (consume == 1) 
                 {                                              /* See if we need to consume the flags                  */
                     p_grp->Flags |= flags_rdy;                 /* Set ONLY the flags that we got                       */
                 }
                 OSTCBCurPtr->FlagsRdy = flags_rdy;             /* Save flags that were ready                           */

                 CPU_CRITICAL_EXIT();                           /* Yes, condition met, return to caller                 */
                *p_err = OS_ERR_NONE;
                 return (flags_rdy);
             } 
             else 
             {                                           /* Block task until events occur or timeout             */
                 if ((opt & OS_OPT_PEND_NON_BLOCKING) != 0u) 
                 {
                     CPU_CRITICAL_EXIT();
                    *p_err = OS_ERR_PEND_WOULD_BLOCK;           /* Specified non-blocking so task would block           */
                     return ((OS_FLAGS)0);
                 } 
                 else 
                 {                                              /* Specified blocking so check is scheduler is locked   */
                     if (OSSchedLockNestingCtr > 0u) 
                     {                                          /* See if called with scheduler locked ...        */
                         CPU_CRITICAL_EXIT();
                         *p_err = OS_ERR_SCHED_LOCKED;          /* ... can't PEND when locked                           */
                         return (0u);
                     }
                 }

                 OS_FlagBlock(p_grp,
                              flags,
                              opt,
                              timeout);
                 CPU_CRITICAL_EXIT();
             }
             break;
        default:
             CPU_CRITICAL_EXIT();
            *p_err = OS_ERR_OPT_INVALID;
             return (0u);
    }

    OSSched();                                                  /* Find next HPT ready to run                           */

    CPU_CRITICAL_ENTER();
    switch (OSTCBCurPtr->PendStatus) 
    {
        case OS_STATUS_PEND_OK:                                 /* We got the event flags                               */
            *p_err = OS_ERR_NONE;
             break;

        case OS_STATUS_PEND_ABORT:                              /* Indicate that we aborted                             */
             CPU_CRITICAL_EXIT();
            *p_err = OS_ERR_PEND_ABORT;
             break;

        case OS_STATUS_PEND_TIMEOUT:                            /* Indicate that we didn't get semaphore within timeout */
             if (p_ts != 0) 
             {
                *p_ts = 0u;
             }
             CPU_CRITICAL_EXIT();
            *p_err = OS_ERR_TIMEOUT;
             break;

        case OS_STATUS_PEND_DEL:                                /* Indicate that object pended on has been deleted      */

             CPU_CRITICAL_EXIT();

            *p_err = OS_ERR_OBJ_DEL;
             break;

        default:
             CPU_CRITICAL_EXIT();
            *p_err = OS_ERR_STATUS_INVALID;
             break;
    }
    
    if (*p_err != OS_ERR_NONE) 
    {
        return (0u);
    }
    
    flags_rdy = OSTCBCurPtr->FlagsRdy;
    
    if (consume == 1) 
    {                                                           /* See if we need to consume the flags                  */
        switch (mode) 
        {
            case OS_OPT_PEND_FLAG_SET_ALL:
            case OS_OPT_PEND_FLAG_SET_ANY:                      /* Clear ONLY the flags we got                          */
                 p_grp->Flags &= ~flags_rdy;
                 break;
            case OS_OPT_PEND_FLAG_CLR_ALL:
            case OS_OPT_PEND_FLAG_CLR_ANY:                      /* Set   ONLY the flags we got                          */
                 p_grp->Flags |=  flags_rdy;
                 break;
            default:
                 CPU_CRITICAL_EXIT();
                *p_err = OS_ERR_OPT_INVALID;
                 return (0u);
        }
    }
    CPU_CRITICAL_EXIT();
   *p_err = OS_ERR_NONE;                                        /* Event(s) must have occurred                          */
    return (flags_rdy);
}

void   OS_FlagTaskRdy (OS_TCB    *p_tcb,
                       OS_FLAGS   flags_rdy,
                       CPU_TS     ts)
{
    (void)ts;                                                   /* Prevent compiler warning for not using 'ts'          */

    p_tcb->FlagsRdy   = flags_rdy;
    p_tcb->PendStatus = OS_STATUS_PEND_OK;                      /* Clear pend status                                    */
    p_tcb->PendOn     = OS_TASK_PEND_ON_NOTHING;                /* Indicate no longer pending                           */

    switch (p_tcb->TaskState) 
    {
        case OS_TASK_STATE_PEND:
        case OS_TASK_STATE_PEND_TIMEOUT:

             if (p_tcb->TaskState == OS_TASK_STATE_PEND_TIMEOUT) 
             {
                 OS_TickListRemove(p_tcb);                      /* Remove from tick list                                */
             }

             OS_RdyListInsert(p_tcb);                           /* Insert the task in the ready list                    */
             p_tcb->TaskState = OS_TASK_STATE_RDY;
             break;

        case OS_TASK_STATE_PEND_SUSPENDED:
        case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:
             p_tcb->TaskState = OS_TASK_STATE_SUSPENDED;
             break;

        case OS_TASK_STATE_RDY:
        case OS_TASK_STATE_DLY:
        case OS_TASK_STATE_DLY_SUSPENDED:
        case OS_TASK_STATE_SUSPENDED:
        default:
             break;
    }
    OS_PendListRemove(p_tcb);
}


OS_FLAGS  OSFlagPost (OS_FLAG_GRP  *p_grp,
                      OS_FLAGS      flags,
                      OS_OPT        opt,
                      OS_ERR       *p_err)
{

    OS_FLAGS       flags_cur;
    OS_FLAGS       flags_rdy;
    OS_OPT         mode;
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_tcb;
    OS_TCB        *p_tcb_next;
    CPU_TS         ts;
    CPU_SR_ALLOC();

    ts = 0u;

    switch (opt) 
    {
        case OS_OPT_POST_FLAG_SET:
        case OS_OPT_POST_FLAG_SET | OS_OPT_POST_NO_SCHED:
             CPU_CRITICAL_ENTER();
             p_grp->Flags |=  flags;                            /* Set   the flags specified in the group               */
             break;

        case OS_OPT_POST_FLAG_CLR:
        case OS_OPT_POST_FLAG_CLR | OS_OPT_POST_NO_SCHED:
             CPU_CRITICAL_ENTER();
             p_grp->Flags &= ~flags;                            /* Clear the flags specified in the group               */
             break;

        default:
            *p_err = OS_ERR_OPT_INVALID;                        /* INVALID option                                       */
             return (0u);
    }
    
    p_pend_list = &p_grp->PendList;
    if (p_pend_list->HeadPtr == 0) 
    {                     
        CPU_CRITICAL_EXIT();                                    /* No                                                   */
       *p_err = OS_ERR_NONE;
        return (p_grp->Flags);
    }

    p_tcb = p_pend_list->HeadPtr;
    while (p_tcb != 0) 
    {                                
        p_tcb_next = p_tcb->PendNextPtr;
        mode       = p_tcb->FlagsOpt & OS_OPT_PEND_FLAG_MASK;
        
        switch (mode) 
        {
            case OS_OPT_PEND_FLAG_SET_ALL:                      /* See if all req. flags are set for current node       */
                 flags_rdy = (p_grp->Flags & p_tcb->FlagsPend);
                 if (flags_rdy == p_tcb->FlagsPend) 
                 {
                     OS_FlagTaskRdy(p_tcb,                      /* Make task RTR, event(s) Rx'd                         */
                                    flags_rdy,
                                    ts);
                 }
                 break;

            case OS_OPT_PEND_FLAG_SET_ANY:                      /* See if any flag set                                  */
                 flags_rdy = (p_grp->Flags & p_tcb->FlagsPend);
                 if (flags_rdy != 0u) 
                 {
                     OS_FlagTaskRdy(p_tcb,                      /* Make task RTR, event(s) Rx'd                         */
                                    flags_rdy,
                                    ts);
                 }
                 break;


            case OS_OPT_PEND_FLAG_CLR_ALL:                      /* See if all req. flags are set for current node       */
                 flags_rdy = (OS_FLAGS)(~p_grp->Flags & p_tcb->FlagsPend);
                 if (flags_rdy == p_tcb->FlagsPend) 
                 {
                     OS_FlagTaskRdy(p_tcb,                      /* Make task RTR, event(s) Rx'd                         */
                                    flags_rdy,
                                    ts);
                 }
                 break;

            case OS_OPT_PEND_FLAG_CLR_ANY:                      /* See if any flag set                                  */
                 flags_rdy = (OS_FLAGS)(~p_grp->Flags & p_tcb->FlagsPend);
                 if (flags_rdy != 0u) 
                 {
                     OS_FlagTaskRdy(p_tcb,                      /* Make task RTR, event(s) Rx'd                         */
                                    flags_rdy,
                                    ts);
                 }
                 break;
            default:
                 CPU_CRITICAL_EXIT();
                *p_err = OS_ERR_FLAG_PEND_OPT;
                 return (0u);
        }
                                                                /* Point to next task waiting for event flag(s)         */
        p_tcb = p_tcb_next;
    }
    CPU_CRITICAL_EXIT();

    if ((opt & OS_OPT_POST_NO_SCHED) == 0u) 
    {
        OSSched();
    }

    CPU_CRITICAL_ENTER();
    flags_cur = p_grp->Flags;
    CPU_CRITICAL_EXIT();
    *p_err     = OS_ERR_NONE;

    return (flags_cur);
}
