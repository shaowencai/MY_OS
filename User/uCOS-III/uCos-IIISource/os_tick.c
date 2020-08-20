#include "os.h"


/* ��ʼ��ʱ���б�������� */
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




/* ��һ��������뵽ʱ���б�������ʱʱ��Ĵ�С�������� */
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
    
    /* ���뵽 OSCfg_TickWheel[spoke] �ĵ�һ���ڵ� */
    if (p_spoke->NbrEntries == (OS_OBJ_QTY)0u) 
    {
        p_tcb->TickNextPtr = (OS_TCB *)0;
        p_tcb->TickPrevPtr = (OS_TCB *)0;
        p_spoke->FirstPtr = p_tcb;
        p_spoke->NbrEntries = (OS_OBJ_QTY)1u;
    }
    /* �������Ĳ��ǵ�һ���ڵ㣬����TickRemain��С�������� */
    else
    {
        /* ��ȡ��һ���ڵ�ָ�� */
        p_tcb1 = p_spoke->FirstPtr;
        while (p_tcb1 != (OS_TCB *)0)
        {
            /* ����ȽϽڵ��ʣ��ʱ�� */
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
                    p_tcb1 = (OS_TCB *)0;//�˳�
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
        /* �ڵ�ɹ����� */
        p_spoke->NbrEntries++;
    }
    
    /* ˢ��NbrEntriesMax��ֵ */
    if (p_spoke->NbrEntriesMax < p_spoke->NbrEntries) 
    {
        p_spoke->NbrEntriesMax = p_spoke->NbrEntries;
    }
    
    /* ����TCB�е�TickSpokePtr��ָ���ڵ� */
    p_tcb->TickSpokePtr = p_spoke;
    
}


void OS_TickListRemove (OS_TCB *p_tcb)
{
    OS_TICK_SPOKE *p_spoke;
    OS_TCB *p_tcb1;
    OS_TCB *p_tcb2;
    
    /* ��ȡ����TCB��������ĸ�ָ�� */
    p_spoke = p_tcb->TickSpokePtr;
    
    if (p_spoke != (OS_TICK_SPOKE *)0)
    {
        p_tcb->TickRemain = (OS_TICK)0u;
        
        /* Ҫ�Ƴ��ĸպ��ǵ�һ���ڵ� */
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
            
            /* �ڵ��Ƴ������ڵ�ǰ��������ڵ�������һ�� */
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
    /* ��ʱ���б�ɾ�� */
    OS_TickListRemove(p_tcb);
    /* ��������б� */
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
    
    /* �����ٽ�� */
    CPU_INT_DIS();
    
    /* ʱ��������++ */
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

                case OS_TASK_STATE_DLY:                               //�������ʱ״̬
                     p_tcb->TickRemain = p_tcb->TickCtrMatch          //������ʱ�ĵ�ʣ��ʱ�� 
                                       - OSTickCtr;
                     if (OSTickCtr == p_tcb->TickCtrMatch) 
                     {        
                         p_tcb->TaskState = OS_TASK_STATE_RDY;        //�޸�����״̬��Ϊ����״̬
                         OS_TaskRdy(p_tcb);                           //���������
                     } 
                     else 
                     {                                   
                         done             = 1;              
                     }
                     break;

                case OS_TASK_STATE_PEND_TIMEOUT:                      //����������޵ȴ�״̬
                case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:            //����������޵ȴ��б�����״̬
                    
                     p_tcb->TickRemain = p_tcb->TickCtrMatch          //�������޵ĵ�ʣ��ʱ��
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
                            //����ȴ�����������������ʱ����ô�Ͳ��ٵȴ����������պ������������ȼ�����ߵģ���ô
                            //ӵ�иû�����������͵ȴ��û���������������ȼ�Ҳ�����µ���
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

                case OS_TASK_STATE_DLY_SUSPENDED:                     //�������ʱ�б�����״̬
                     p_tcb->TickRemain = p_tcb->TickCtrMatch          //������ʱ�ĵ�ʣ��ʱ�� 
                                       - OSTickCtr;
                     if (OSTickCtr == p_tcb->TickCtrMatch) 
                     {     
                         p_tcb->TaskState  = OS_TASK_STATE_SUSPENDED; //�޸�����״̬��Ϊ������״̬
                         OS_TickListRemove(p_tcb);                    //�ӽ����б��Ƴ�������
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
    
    CPU_INT_EN();/* �˳��ٽ�� */
 
}

