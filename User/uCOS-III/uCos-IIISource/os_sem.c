#include "os.h"



void  OS_SemClr (OS_SEM  *p_sem)
{
    p_sem->Type    = 0;                                     /* Mark the data structure as a NONE                      */
    p_sem->Ctr     = (OS_SEM_CTR)0;                         /* Set semaphore value                                    */
    p_sem->TS      = (CPU_TS    )0;                         /* Clear the time stamp                                   */
    p_sem->NamePtr = (CPU_CHAR *)((void *)"?SEM");
    OS_PendListInit(&p_sem->PendList);                      /* Initialize the waiting list                            */
}




void  OSSemCreate (OS_SEM      *p_sem,  //��ֵ�ź������ƿ�ָ��
                   CPU_CHAR    *p_name, //��ֵ�ź�������
                   OS_SEM_CTR   cnt,    //��Դ��Ŀ���¼��Ƿ�����־
                   OS_ERR      *p_err)  //���ش�������
{
    CPU_SR_ALLOC(); //ʹ�õ��ٽ�Σ��ڹ�/���ж�ʱ��ʱ����ú꣬�ú������Ͷ���һ���ֲ���
                    //�������ڱ�����ж�ǰ�� CPU ״̬�Ĵ��� SR���ٽ�ι��ж�ֻ�豣��SR��
                    //�����ж�ʱ����ֵ��ԭ�� 

    CPU_CRITICAL_ENTER();               //�����ٽ��
    p_sem->Type    = 0;  
    p_sem->Ctr     = cnt;                                 
    p_sem->TS      = (CPU_TS)0;
    p_sem->NamePtr = p_name;                               
    OS_PendListInit(&p_sem->PendList);   //��ʼ���ö�ֵ�ź����ĵȴ��б�     

    OSSemQty++;                         //��ֵ�ź���������1

    CPU_CRITICAL_EXIT();                //�˳��ٽ�Σ��޵��ȣ�
   *p_err = OS_ERR_NONE;                 //��������Ϊ���޴���
}



OS_OBJ_QTY  OSSemDel (OS_SEM  *p_sem,  //��ֵ�ź���ָ��
                      OS_OPT   opt,    //ѡ��
                      OS_ERR  *p_err)  //���ش�������
{
    OS_OBJ_QTY     nbr_tasks;
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_tcb;
    CPU_TS         ts;
    
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();                      //���ж�
    p_pend_list = &p_sem->PendList;            //��ȡ�ź����ĵȴ��б� p_pend_list
    nbr_tasks = 0;
    switch (opt) 
    {                           
        case OS_OPT_DEL_NO_PEND:               //���ֻ��û������ȴ��������ɾ���ź���
        {
             if (p_pend_list->HeadPtr == 0)
             {
                 OSSemQty--;                   //�ź�����Ŀ��1
                 OS_SemClr(p_sem);             //����ź�������
                 CPU_CRITICAL_EXIT();          //���ж�
                *p_err = OS_ERR_NONE;          //���ش�������Ϊ���޴���
             } 
             else 
             {                         
                 CPU_CRITICAL_EXIT();          //���ж�
                *p_err = OS_ERR_TASK_WAITING;  //���ش�������Ϊ���������ڵȴ����ź�����
             }
             break;
         }

        case OS_OPT_DEL_ALWAYS:                             //�������ɾ���ź���
        {
            
             while (p_pend_list->HeadPtr != 0)
             {                                              //����Ƴ����ź����ȴ��б��е�����
                 p_tcb       = p_pend_list->HeadPtr;
                 OS_PendObjDel((OS_PEND_OBJ *)((void *)p_sem),p_tcb,ts);
                 nbr_tasks++;
             }
             
             OSSemQty--;                                    //�ź�����Ŀ��1
             OS_SemClr(p_sem);                              //����ź�������
             CPU_CRITICAL_EXIT();                           //�������������������е���
             OSSched();                                     //������ȣ�ִ��������ȼ��ľ�������
            *p_err = OS_ERR_NONE;                           //���ش�������Ϊ���޴���
             break;
        }
        default: 
        {
             CPU_CRITICAL_EXIT();                           //���ж�
            *p_err = OS_ERR_OPT_INVALID;                    //���ش�������Ϊ��ѡ��Ƿ���
             break;
        }
    }
    return ((OS_OBJ_QTY)nbr_tasks);                         //����ɾ���ź���ǰ�ȴ����������
}




OS_SEM_CTR  OSSemPend (OS_SEM   *p_sem,   //��ֵ�ź���ָ��
                       OS_TICK   timeout, //�ȴ���ʱʱ��
                       OS_OPT    opt,     //ѡ��
                       CPU_TS   *p_ts,    //�ȵ��ź���ʱ��ʱ���
                       OS_ERR   *p_err)   //���ش�������
{
    OS_SEM_CTR    ctr;
    CPU_SR_ALLOC();
    
    CPU_CRITICAL_ENTER();                     //���ж�
    
    if (p_sem->Ctr > (OS_SEM_CTR)0) 
    {       
        p_sem->Ctr--;                         //��Դ��Ŀ��1
        ctr   = p_sem->Ctr;                   //��ȡ�ź����ĵ�ǰ��Դ��Ŀ
        CPU_CRITICAL_EXIT();                  //���ж�
       *p_err = OS_ERR_NONE;                  //���ش�������Ϊ���޴���
        return (ctr);                         //�����ź����ĵ�ǰ��Դ��Ŀ��������ִ��
    }
    
    if ((opt & OS_OPT_PEND_NON_BLOCKING) != (OS_OPT)0) 
    { 
        ctr   = p_sem->Ctr;                                 //��ȡ�ź�������Դ��Ŀ�� ctr
        CPU_CRITICAL_EXIT();                                //���ж�
       *p_err = OS_ERR_PEND_WOULD_BLOCK;                    //���ش�������Ϊ���ȴ����������  
        return (ctr);                                       //�����ź����ĵ�ǰ��Դ��Ŀ��������ִ��
    } 
                                                   
    OS_Pend((OS_PEND_OBJ *)((void *)p_sem), OS_TASK_PEND_ON_SEM,timeout);

    CPU_CRITICAL_EXIT();            

    OSSched();                                              //�ҵ�������������ȼ���������

    CPU_CRITICAL_ENTER();
    ctr = p_sem->Ctr;                                       //��ȡ�ź����ĵ�ǰ��Դ��Ŀ
    CPU_CRITICAL_EXIT();                                    //���ж�
    return (ctr);                                           //�����ź����ĵ�ǰ��Դ��Ŀ
}



void  OS_SemInit (OS_ERR  *p_err)
{
    OSSemQty        = (OS_OBJ_QTY)0;
    *p_err          = OS_ERR_NONE;
}



OS_SEM_CTR  OS_SemPost (OS_SEM  *p_sem, //��ֵ�ź���ָ��
                        OS_OPT   opt,   //ѡ��
                        CPU_TS   ts,    //ʱ���
                        OS_ERR  *p_err) //���ش�������
{
    OS_OBJ_QTY     cnt;
    OS_SEM_CTR     ctr;
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_tcb;
    OS_TCB        *p_tcb_next;
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();                                   //���ж�
    
    p_pend_list = &p_sem->PendList;                         //ȡ�����ź����ĵȴ��б�
    
    if (p_pend_list->HeadPtr == 0) 
    {                      
                          
        if ((int)(p_sem->Ctr) == -1) 
        {  
            CPU_CRITICAL_EXIT();        
            *p_err = OS_ERR_SEM_OVF;
            return ((OS_SEM_CTR)0);
        }

        p_sem->Ctr++;                                       //�ź�������ֵ��������1
        ctr       = p_sem->Ctr;                             //��ȡ�ź�������ֵ�� ctr
        p_sem->TS = ts;                                     //����ʱ���
        CPU_CRITICAL_EXIT();                                //���ж�
       *p_err     = OS_ERR_NONE;                            //���ش�������Ϊ���޴���
        return (ctr);                                       //�����ź����ļ���ֵ��������ִ��
    }

    if ((opt & OS_OPT_POST_ALL) != (OS_OPT)0) 
    {                                                       //���Ҫ���ź������������еȴ�����
        cnt = p_pend_list->NbrEntries;                      //��ȡ�ȴ�������Ŀ�� cnt
    } 
    else 
    {                                                       //���Ҫ���ź������������ȼ���ߵĵȴ�����
        cnt = (OS_OBJ_QTY)1;                                //��Ҫ������������Ϊ1��cnt ��1
    }
    
    p_tcb = p_pend_list->HeadPtr;                           //�ȴ������е������ǰ����ȼ������ ���Ե�һ���ǵȴ������ȼ���ߵ�
    
    while (cnt > 0u) 
    {                                                    
        p_tcb_next = p_tcb->PendNextPtr;            
        
        OS_Post((OS_PEND_OBJ *)((void *)p_sem),             //�����ź�������ǰ����
                p_tcb,
                (void      *)0,
                (OS_MSG_SIZE)0,
                ts);
        p_tcb = p_tcb_next;       
        cnt--;
    }
    
    ctr = p_sem->Ctr;                                       //��ȡ�ź�������ֵ�� ctr
    
    CPU_CRITICAL_EXIT();                                    //����������������ִ���������
    
    if ((opt & OS_OPT_POST_NO_SCHED) == (OS_OPT)0)          //��� opt ûѡ�񡰷���ʱ����������
    {       
        OSSched();                                          //�������
    }
    
    *p_err = OS_ERR_NONE;                                   //���ش�������Ϊ���޴���
    return (ctr);                                           //�����ź����ĵ�ǰ����ֵ
}


OS_SEM_CTR  OSSemPost (OS_SEM  *p_sem,    //��ֵ�ź������ƿ�ָ��
                       OS_OPT   opt,      //ѡ��
                       OS_ERR  *p_err)    //���ش�������
{
    OS_SEM_CTR  ctr;
    CPU_TS      ts;

    ctr = OS_SemPost(p_sem,opt,ts,p_err);

    return (ctr);                                 //�����źŵĵ�ǰ����ֵ
}
