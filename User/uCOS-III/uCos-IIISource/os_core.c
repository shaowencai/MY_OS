#include "os.h"

static void OS_RdyListInit(void)
{
    OS_PRIO i; 
    OS_RDY_LIST *p_rdy_list; 

    /* ѭ����ʼ�������г�Ա����ʼ��Ϊ0 */ 
    for ( i=0u; i<OS_CFG_PRIO_MAX; i++ ) 
    { 
        p_rdy_list = &OSRdyList[i]; 
        p_rdy_list->NbrEntries = (OS_OBJ_QTY)0; 
        p_rdy_list->HeadPtr = (OS_TCB *)0; 
        p_rdy_list->TailPtr = (OS_TCB *)0; 
    }
}


void OS_RdyListInsertHead (OS_TCB *p_tcb)
{
    OS_RDY_LIST *p_rdy_list;
    OS_TCB *p_tcb2;

    /* ��ȡ������� */ 
    p_rdy_list = &OSRdyList[p_tcb->Prio]; 

    /* CASE 0: �����ǿ����� */ 
    if (p_rdy_list->NbrEntries == (OS_OBJ_QTY)0) 
    { 
        p_rdy_list->NbrEntries = (OS_OBJ_QTY)1; 
        p_tcb->NextPtr = (OS_TCB *)0; 
        p_tcb->PrevPtr = (OS_TCB *)0; 
        p_rdy_list->HeadPtr = p_tcb; 
        p_rdy_list->TailPtr = p_tcb; 
    } 
    /* CASE 1: �������нڵ� */ 
    else 
    { 
        p_rdy_list->NbrEntries++; 
        p_tcb->NextPtr = p_rdy_list->HeadPtr; 
        p_tcb->PrevPtr = (OS_TCB *)0; 
        p_tcb2 = p_rdy_list->HeadPtr; 
        p_tcb2->PrevPtr = p_tcb; 
        p_rdy_list->HeadPtr = p_tcb; 
    }
}

void OS_RdyListInsertTail (OS_TCB *p_tcb)
{
    OS_RDY_LIST *p_rdy_list;
    OS_TCB *p_tcb2;

    p_rdy_list = &OSRdyList[p_tcb->Prio];
    /* CASE 0: �����ǿ����� */ 
    if (p_rdy_list->NbrEntries == (OS_OBJ_QTY)0) 
    { 
        p_rdy_list->NbrEntries = (OS_OBJ_QTY)1; 
        p_tcb->NextPtr = (OS_TCB *)0; 
        p_tcb->PrevPtr = (OS_TCB *)0; 
        p_rdy_list->HeadPtr = p_tcb; 
        p_rdy_list->TailPtr = p_tcb; 
    }
    else
    {
        p_rdy_list->NbrEntries++; 
        p_tcb->NextPtr  = 0;
        p_tcb2          = p_rdy_list->TailPtr; 
        p_tcb->PrevPtr  = p_tcb2; 
        p_tcb2->NextPtr = p_tcb;
        p_rdy_list->TailPtr = p_tcb;
    }
}


void OS_RdyListRemove (OS_TCB *p_tcb) 
{  
    OS_RDY_LIST *p_rdy_list; 
        
    OS_TCB *p_tcb1; 
    OS_TCB *p_tcb2;
    
    p_rdy_list = &OSRdyList[p_tcb->Prio];
    p_tcb1 = p_tcb->PrevPtr;
    p_tcb2 = p_tcb->NextPtr;
    
    if (p_tcb1 == (OS_TCB *)0)
    {
        if (p_tcb2 == (OS_TCB *)0) 
        {
            p_rdy_list->NbrEntries = (OS_OBJ_QTY)0;
            p_rdy_list->HeadPtr = (OS_TCB *)0;
            p_rdy_list->TailPtr = (OS_TCB *)0;
            OS_PrioRemove(p_tcb->Prio);
        }
        else
        {
            p_rdy_list->NbrEntries--;
            p_tcb2->PrevPtr = (OS_TCB *)0;
            p_rdy_list->HeadPtr = p_tcb2;
        }
    }
    else
    {
        p_rdy_list->NbrEntries--;
        p_tcb1->NextPtr = p_tcb2;
        /* ���Ҫɾ���Ľڵ����һ���ڵ���0����Ҫɾ���Ľڵ������һ���ڵ� */
        if (p_tcb2 == (OS_TCB *)0)
        {
            p_rdy_list->TailPtr = p_tcb1;
        }
        else
        {
            p_tcb2->PrevPtr = p_tcb1;
        }
    }
    
    p_tcb->PrevPtr = (OS_TCB *)0;
    p_tcb->NextPtr = (OS_TCB *)0;
}


void OS_RdyListMoveHeadToTail (OS_RDY_LIST *p_rdy_list)
{
    OS_TCB *p_tcb1; 
    OS_TCB *p_tcb2;
    OS_TCB *p_tcb3; 
    
    switch (p_rdy_list->NbrEntries) 
    {
        case 0:
        case 1:
            break;
        case 2:
            p_tcb1 = p_rdy_list->HeadPtr;
            p_tcb2 = p_rdy_list->TailPtr;
            p_tcb1->PrevPtr = p_tcb2;
            p_tcb1->NextPtr = (OS_TCB *)0;
            p_tcb2->PrevPtr = (OS_TCB *)0;
            p_tcb2->NextPtr = p_tcb1;
            p_rdy_list->HeadPtr = p_tcb2;
            p_rdy_list->TailPtr = p_tcb1;
            break;
        default :
            p_tcb1 = p_rdy_list->HeadPtr;
            p_tcb2 = p_rdy_list->TailPtr;
            p_tcb3 = p_tcb1->NextPtr;
            p_tcb3->PrevPtr = (OS_TCB *)0;
            p_tcb1->NextPtr = (OS_TCB *)0;
            p_tcb1->PrevPtr = p_tcb2;
            p_tcb1->PrevPtr = p_tcb2;
            p_rdy_list->HeadPtr = p_tcb3;
            p_rdy_list->TailPtr = p_tcb1;
            break;
        }
}

void OS_RdyListInsert (OS_TCB *p_tcb)
{
    OS_PrioInsert(p_tcb->Prio);
    
    if (p_tcb->Prio == OSPrioCur) 
    {
        OS_RdyListInsertTail(p_tcb);
    }
    else
    {  
        OS_RdyListInsertHead(p_tcb);
    }
}



void OS_IdleTaskInit(OS_ERR *p_err)
{
    OSIdleTaskCtr = (OS_IDLE_CTR)0;
    /* ������������ */ 
    OSTaskCreate ((OS_TCB*) &OSIdleTaskTCB, 
                 (OS_TASK_PTR ) OS_IdleTask, 
                 (void *) 0, 
                 (OS_PRIO)(31),
                 (CPU_STK*) &OSCfg_IdleTaskStkBasePtr[0], 
                 (CPU_STK_SIZE) OSCfg_IdleTaskStkSize, 
                 (OS_TICK) 0,
                 (OS_ERR *) p_err);
    
}



void OSInit (OS_ERR *p_err)
{  
    OSRunning = OS_STATE_OS_STOPPED;
    OSTCBCurPtr = (OS_TCB *)0; 
    OSTCBHighRdyPtr = (OS_TCB *)0;

    OSPrioCur = (OS_PRIO)0;
    OSPrioHighRdy = (OS_PRIO)0;

    OS_TmrInit(p_err);
    OS_SemInit(p_err);
    
    OS_PrioInit();
    OS_RdyListInit();

    /* ��ʼ���������� */
    OS_IdleTaskInit(p_err);
    if (*p_err != OS_ERR_NONE)
    {
        return;
    }
    
    *p_err = OS_ERR_NONE;
}


void OSStart (OS_ERR *p_err)
{
    if ( OSRunning == OS_STATE_OS_STOPPED )
    {
        OSRunning  = OS_STATE_OS_RUNNING;
        #if 0
        OSTCBHighRdyPtr = OSRdyList[0].HeadPtr;
        #endif
        
        OSPrioHighRdy = OS_PrioGetHighest();
        OSPrioCur = OSPrioHighRdy;

        OSTCBHighRdyPtr = OSRdyList[OSPrioHighRdy].HeadPtr;
        OSTCBCurPtr = OSTCBHighRdyPtr;

        OSStartHighRdy();
        
        *p_err = OS_ERR_FATAL_RETURN;
    }
    else
    {
        *p_err = OS_ERR_OS_RUNNING;
    }
}

void OSSched (void) 
{  
#if 0
    if ( OSTCBCurPtr == OSRdyList[0].HeadPtr )
    { 
        OSTCBHighRdyPtr = OSRdyList[1].HeadPtr;
    } 
    else 
    {  
        OSTCBHighRdyPtr = OSRdyList[0].HeadPtr; 
    } 
    OS_TASK_SW();
#endif

#if 0
    if ( OSTCBCurPtr == &OSIdleTaskTCB )
    {
        if(OSRdyList[0].HeadPtr->TaskDelayTicks == 0)
        {
            OSTCBHighRdyPtr = OSRdyList[0].HeadPtr;
        }
        else if(OSRdyList[1].HeadPtr->TaskDelayTicks == 0)
        {
            OSTCBHighRdyPtr = OSRdyList[1].HeadPtr;
        }
        else
        {
            return;
        }
    }
    else
    {
        if (OSTCBCurPtr == OSRdyList[0].HeadPtr) 
        {
            if (OSRdyList[1].HeadPtr->TaskDelayTicks == 0)
            {
                OSTCBHighRdyPtr = OSRdyList[1].HeadPtr;
            }
            else if (OSTCBCurPtr->TaskDelayTicks != 0)
            {
                OSTCBHighRdyPtr = &OSIdleTaskTCB;
            }
            else
            {
                return;
            }
        }
        else if (OSTCBCurPtr == OSRdyList[1].HeadPtr) 
        {
            if (OSRdyList[0].HeadPtr->TaskDelayTicks == 0)
            {
                OSTCBHighRdyPtr = OSRdyList[0].HeadPtr;
            }
            else if (OSTCBCurPtr->TaskDelayTicks != 0)
            {
                OSTCBHighRdyPtr = &OSIdleTaskTCB;
            }
            else
            {
                return;
            }
        }
    }
#endif    
    
    CPU_SR_ALLOC();
   
    CPU_INT_DIS();
    
    OSPrioHighRdy = OS_PrioGetHighest();
    OSTCBHighRdyPtr = OSRdyList[OSPrioHighRdy].HeadPtr;
    if (OSTCBHighRdyPtr == OSTCBCurPtr) 
    {
        CPU_INT_EN();
        return;
    }
    
    CPU_INT_EN();
    OS_TASK_SW();

}



#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u
void OS_SchedRoundRobin(OS_RDY_LIST *p_rdy_list)
{
    OS_TCB *p_tcb;
    CPU_SR_ALLOC();
    
    CPU_INT_DIS();
    p_tcb = p_rdy_list->HeadPtr;
    
    if (p_tcb == (OS_TCB *)0)
    {
        CPU_INT_EN();
        return;        
    }
    
    if (p_tcb == &OSIdleTaskTCB)
    {
        CPU_INT_EN();
        return;           
    }
    
    if (p_tcb->TimeQuantaCtr > (OS_TICK)0)
    {
      p_tcb->TimeQuantaCtr--;  
    }
    
    /* ʱ��Ƭû�����꣬���˳� */
    if (p_tcb->TimeQuantaCtr > (OS_TICK)0)
    {
        CPU_INT_EN();
        return;          
    }
    
    /* �����ǰ���ȼ�ֻ��һ���������˳� */
    if (p_rdy_list->NbrEntries < (OS_OBJ_QTY)2)
    {
        CPU_INT_EN();
        return;          
    }
    
    /* ʱ��Ƭ���꣬������ŵ���������һ���ڵ� */
    OS_RdyListMoveHeadToTail(p_rdy_list);
    
    p_tcb = p_rdy_list->HeadPtr;
    p_tcb->TimeQuantaCtr = p_tcb->TimeQuanta;
    
    CPU_INT_EN();/* �˳��ٽ�� */
}
#endif

void  OS_PendListInit (OS_PEND_LIST  *p_pend_list)
{
    p_pend_list->HeadPtr    = (OS_TCB *)0;   //��λ�ȴ��б�����г�Ա
    p_pend_list->TailPtr    = (OS_TCB *)0;
    p_pend_list->NbrEntries = (OS_OBJ_QTY    )0;
}




void  OS_PendListRemove (OS_TCB  *p_tcb)
{
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_next;
    OS_TCB        *p_prev;


    if (p_tcb->PendObjPtr != 0) 
    {                       
        p_pend_list = &p_tcb->PendObjPtr->PendList;             

        if (p_pend_list->HeadPtr->PendNextPtr == 0) 
        {
            p_pend_list->HeadPtr = 0;              
            p_pend_list->TailPtr = 0;
        } 
        else if (p_tcb->PendPrevPtr == 0) 
        {          
            p_next               = p_tcb->PendNextPtr;       
            p_next->PendPrevPtr  = 0;
            p_pend_list->HeadPtr = p_next;

        } 
        else if (p_tcb->PendNextPtr == 0) 
        {          
            p_prev               = p_tcb->PendPrevPtr;   
            p_prev->PendNextPtr  = 0;
            p_pend_list->TailPtr = p_prev;

        } 
        else 
        {
            p_prev               = p_tcb->PendPrevPtr;      
            p_next               = p_tcb->PendNextPtr;
            p_prev->PendNextPtr  = p_next;
            p_next->PendPrevPtr  = p_prev;
        }
        
        p_pend_list->NbrEntries--;

        p_tcb->PendNextPtr = 0;
        p_tcb->PendPrevPtr = 0;
        p_tcb->PendObjPtr  = 0;
    }
}

void  OS_PendObjDel (OS_PEND_OBJ  *p_obj,  //��ɾ�����������
                     OS_TCB       *p_tcb,  //������ƿ�ָ��
                     CPU_TS        ts)     //�ź�����ɾ��ʱ��ʱ���
{
    switch (p_tcb->TaskState)
    {                         
        case OS_TASK_STATE_RDY:                             //��������Ǿ���״̬
        case OS_TASK_STATE_DLY:                             //�����������ʱ״̬
        case OS_TASK_STATE_SUSPENDED:                       //��������ǹ���״̬
        case OS_TASK_STATE_DLY_SUSPENDED:                   //�������������ʱ�б�����
             break;                                         //��Щ�������ȴ��޹أ�ֱ������

        case OS_TASK_STATE_PEND:                            //��������������޵ȴ�״̬
        case OS_TASK_STATE_PEND_TIMEOUT:                    //��������������޵ȴ�״̬

             OS_PendListRemove(p_tcb);                    //����������еȴ��б����Ƴ�
             OS_TaskRdy(p_tcb);                           //�������׼������
             p_tcb->TaskState  = OS_TASK_STATE_RDY;       //�޸�����״̬Ϊ����״̬
             p_tcb->PendStatus = OS_STATUS_PEND_DEL;      //�������ĵȴ�����ɾ��          
             p_tcb->PendOn     = OS_TASK_PEND_ON_NOTHING; //�������Ŀǰû�еȴ��κζ���
             break;                                       //����

        case OS_TASK_STATE_PEND_SUSPENDED:                //��������������޵ȴ��б�����
        case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:        //��������������޵ȴ��б�����
            
             OS_TickListRemove(p_tcb);                     //��������������б� 
             OS_PendListRemove(p_tcb);                     //����������еȴ��б����Ƴ�
             p_tcb->TaskState  = OS_TASK_STATE_SUSPENDED;  //�޸�����״̬Ϊ����״̬
             p_tcb->PendStatus = OS_STATUS_PEND_DEL;       //�������ĵȴ�����ɾ��    
             p_tcb->PendOn     = OS_TASK_PEND_ON_NOTHING;  //�������Ŀǰû�еȴ��κζ���
             break;                                        //����

        default:                                           //�������״̬����Ԥ��
             break;                                        //���账��ֱ������
    }
}


void  OS_TaskBlock (OS_TCB   *p_tcb,
                    OS_TICK   timeout)
{
    if (timeout > (OS_TICK)0) 
    {                           
        OS_TickListInsert(p_tcb,timeout);
        
        p_tcb->TaskState = OS_TASK_STATE_PEND_TIMEOUT;

    } 
    else 
    {
        p_tcb->TaskState = OS_TASK_STATE_PEND;
    }
    
    OS_RdyListRemove(p_tcb);
}

void  OS_PendListInsertPrio (OS_PEND_LIST  *p_pend_list,
                             OS_TCB        *p_tcb)
{
    OS_PRIO        prio;
    OS_TCB        *p_tcb_next;

                                 
    prio  = p_tcb->Prio;
    
    if (p_pend_list->HeadPtr == 0) 
    {           

        p_pend_list->NbrEntries = 1u;                      
        p_tcb->PendNextPtr   = 0;                        
        p_tcb->PendPrevPtr   = 0;
        p_pend_list->HeadPtr = p_tcb;
        p_pend_list->TailPtr = p_tcb;
    } 
    else 
    {
        p_pend_list->NbrEntries++;                              /* CASE 1: One more OS_TCBs in the list                 */

        p_tcb_next = p_pend_list->HeadPtr;
        while (p_tcb_next != 0) 
        {                    
            if (prio < p_tcb_next->Prio) 
            {
                break;                                          
            } 
            else 
            {
                p_tcb_next = p_tcb_next->PendNextPtr;         
            }
        }
        if (p_tcb_next == 0) 
        {                           
            p_tcb->PendNextPtr              = 0;         
            p_tcb->PendPrevPtr              = p_pend_list->TailPtr;
            p_tcb->PendPrevPtr->PendNextPtr = p_tcb;
            p_pend_list->TailPtr            = p_tcb;
        } 
        else 
        {
            if (p_tcb_next->PendPrevPtr == 0) 
            {         
                p_tcb->PendNextPtr      = p_tcb_next;           
                p_tcb->PendPrevPtr      = 0;
                p_tcb_next->PendPrevPtr = p_tcb;
                p_pend_list->HeadPtr    = p_tcb;
            } 
            else 
            {                                            
                p_tcb->PendNextPtr              = p_tcb_next;
                p_tcb->PendPrevPtr              = p_tcb_next->PendPrevPtr;
                p_tcb->PendPrevPtr->PendNextPtr = p_tcb;
                p_tcb_next->PendPrevPtr         = p_tcb;
            }
        }
    }
}


void  OS_Pend (OS_PEND_OBJ   *p_obj,        //�ȴ����ں˶���
               OS_STATE       pending_on,   //�ȴ����ֶ����ں�
               OS_TICK        timeout)      //�ȴ�����
{
    OS_PEND_LIST  *p_pend_list;



    OSTCBCurPtr->PendOn     = pending_on;                    //��Դ�����ã���ʼ�ȴ�
    OSTCBCurPtr->PendStatus = OS_STATUS_PEND_OK;             //�����ȴ���

    OS_TaskBlock(OSTCBCurPtr,timeout);                       //��� timeout ��0�����������Ľ����б�

    if (p_obj != (OS_PEND_OBJ *)0) 
    {                        
        p_pend_list             = &p_obj->PendList;          //��ȡ����ĵȴ��б� p_pend_list
        OSTCBCurPtr->PendObjPtr = p_obj;
        
        OS_PendListInsertPrio(p_pend_list,                   //�����ȼ��� p_pend_data ���뵽�ȴ��б�
                              OSTCBCurPtr);
    } 
    else                                                    //����ȴ�����Ϊ��
    {                                               
        OSTCBCurPtr->PendObjPtr =  0;  
    }

}

void  OS_Post (OS_PEND_OBJ  *p_obj,     //�ں˶�������ָ��
               OS_TCB       *p_tcb,     //������ƿ�
               void         *p_void,    //��Ϣ
               OS_MSG_SIZE   msg_size,  //��Ϣ��С
               CPU_TS        ts)        //ʱ���
{

    switch (p_tcb->TaskState) 
    {
        case OS_TASK_STATE_RDY:                                 /* Cannot Post a task that is ready                     */
        case OS_TASK_STATE_DLY:                                 /* Cannot Post a task that is delayed                   */
        case OS_TASK_STATE_SUSPENDED:                           /* Cannot Post a suspended task                         */
        case OS_TASK_STATE_DLY_SUSPENDED:                       /* Cannot Post a suspended task that was also dly'd     */
             break;

        case OS_TASK_STATE_PEND:
        case OS_TASK_STATE_PEND_TIMEOUT:

             if (p_obj != 0) 
             {
                 OS_PendListRemove(p_tcb);                      /* Remove task from pend list                           */
             }

             if (p_tcb->TaskState == OS_TASK_STATE_PEND_TIMEOUT) 
             {
                 OS_TickListRemove(p_tcb);                      /* Remove from tick list                                */
             }

             OS_RdyListInsert(p_tcb);                           /* Insert the task in the ready list                    */
             p_tcb->TaskState  = OS_TASK_STATE_RDY;
             p_tcb->PendStatus = OS_STATUS_PEND_OK;             /* Clear pend status                                    */
             p_tcb->PendOn     = OS_TASK_PEND_ON_NOTHING;       /* Indicate no longer pending                           */
             break;

        case OS_TASK_STATE_PEND_SUSPENDED:
        case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:


             if (p_obj != 0) 
             {
                 OS_PendListRemove(p_tcb);                      /* Remove from pend list                                */
             }


             if (p_tcb->TaskState == OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED) 
             {
                 OS_TickListRemove(p_tcb);                      /* Cancel any timeout                                   */
             }

             p_tcb->TaskState  = OS_TASK_STATE_SUSPENDED;
             p_tcb->PendStatus = OS_STATUS_PEND_OK;             /* Clear pend status                                    */
             p_tcb->PendOn     = OS_TASK_PEND_ON_NOTHING;       /* Indicate no longer pending                           */
             
             break;

        default:
             break;
    }
}

//���ı����ȼ���TCB��ԭ���ĵȴ�����ɾ�������°��յ�ǰ���ȼ�����������
void  OS_PendListChangePrio (OS_TCB  *p_tcb)
{
    OS_PEND_LIST  *p_pend_list;
    OS_PEND_OBJ   *p_obj;

    p_obj       =  p_tcb->PendObjPtr;                       /* Get pointer to pend list                             */
    p_pend_list = &p_obj->PendList;

    if (p_pend_list->HeadPtr->PendNextPtr != 0) 
    {      
        OS_PendListRemove(p_tcb);                           /* Remove entry from current position                   */
        p_tcb->PendObjPtr = p_obj;
        OS_PendListInsertPrio(p_pend_list,                  /* INSERT it back in the list                           */
                              p_tcb);
    }
}

void  OS_PendAbort (OS_TCB     *p_tcb,
                    CPU_TS      ts,
                    OS_STATUS   reason)
{


    switch (p_tcb->TaskState) 
    {
        case OS_TASK_STATE_PEND:
        case OS_TASK_STATE_PEND_TIMEOUT:

             OS_PendListRemove(p_tcb);                          /* Remove task from the pend list                       */


             if (p_tcb->TaskState == OS_TASK_STATE_PEND_TIMEOUT) 
             {
                 OS_TickListRemove(p_tcb);                      /* Cancel the timeout                                   */
             }

             OS_RdyListInsert(p_tcb);                           /* Insert the task in the ready list                    */
             p_tcb->TaskState  = OS_TASK_STATE_RDY;             /* Task will be ready                                   */
             p_tcb->PendStatus = reason;                        /* Indicate how the task became ready                   */
             p_tcb->PendOn     = OS_TASK_PEND_ON_NOTHING;       /* Indicate no longer pending                           */
             break;

        case OS_TASK_STATE_PEND_SUSPENDED:
        case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:


             OS_PendListRemove(p_tcb);                          /* Remove task from the pend list                       */

             if (p_tcb->TaskState == OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED) 
             {
                 OS_TickListRemove(p_tcb);                      /* Cancel the timeout                                   */
             }

             p_tcb->TaskState  = OS_TASK_STATE_SUSPENDED;       /* Task needs to remain suspended                       */
             p_tcb->PendStatus = reason;                        /* Indicate how the task became ready                   */
             p_tcb->PendOn     = OS_TASK_PEND_ON_NOTHING;       /* Indicate no longer pending                           */
             break;

        case OS_TASK_STATE_RDY:                                 /* Cannot cancel a pend when a task is in these states. */
        case OS_TASK_STATE_DLY:
        case OS_TASK_STATE_SUSPENDED:
        case OS_TASK_STATE_DLY_SUSPENDED:
        default:
             break;
    }
}



