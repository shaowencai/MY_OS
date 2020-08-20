#include <os.h>


static void  OS_TmrUnlink (OS_TMR  *p_tmr);
static void  OS_TmrLink (OS_TMR  *p_tmr, OS_OPT   opt);
static void  OS_TmrClr (OS_TMR  *p_tmr);

#define  OS_OPT_LINK_DLY        (OS_OPT)(0u)
#define  OS_OPT_LINK_PERIODIC   (OS_OPT)(1u)


void  OSTmrCreate (OS_TMR               *p_tmr,          //��ʱ�����ƿ�ָ��
                   CPU_CHAR             *p_name,         //������ʱ���������ڵ���
                   OS_TICK               dly,            //��ʼ��ʱ������
                   OS_TICK               period,         //���ڶ�ʱ���ؽ�����
                   OS_OPT                opt,            //ѡ��
                   OS_TMR_CALLBACK_PTR   p_callback,     //��ʱ����ʱ�Ļص�����
                   void                 *p_callback_arg, //�����ص������Ĳ���
                   OS_ERR               *p_err)          //���ش�������
{
    CPU_SR_ALLOC(); //ʹ�õ��ٽ�Σ��ڹ�/���ж�ʱ��ʱ����ú꣬�ú������Ͷ���һ���ֲ���
                    //�������ڱ�����ж�ǰ�� CPU ״̬�Ĵ��� SR���ٽ�ι��ж�ֻ�豣��SR��
                    //�����ж�ʱ����ֵ��ԭ�� 

    CPU_CRITICAL_ENTER();         //�����ٽ��
    
    p_tmr->State          = (OS_STATE           )OS_TMR_STATE_STOPPED;  //��ʼ����ʱ��ָ��
    p_tmr->Type           = (OS_OBJ_TYPE        )0;
    p_tmr->NamePtr        = (CPU_CHAR          *)p_name;
    p_tmr->Dly            = (OS_TICK            )dly;
    p_tmr->Match          = (OS_TICK            )0;
    p_tmr->Remain         = (OS_TICK            )0;
    p_tmr->Period         = (OS_TICK            )period;
    p_tmr->Opt            = (OS_OPT             )opt;
    p_tmr->CallbackPtr    = (OS_TMR_CALLBACK_PTR)p_callback;
    p_tmr->CallbackPtrArg = (void              *)p_callback_arg;
    p_tmr->NextPtr        = (OS_TMR            *)0;
    p_tmr->PrevPtr        = (OS_TMR            *)0;

    OSTmrQty++;            

    CPU_CRITICAL_EXIT(); 
    *p_err = OS_ERR_NONE;        
}


CPU_BOOLEAN  OSTmrDel (OS_TMR  *p_tmr, //��ʱ�����ƿ�ָ��
                       OS_ERR  *p_err) //���ش�������
{
    CPU_BOOLEAN  success;  //�ݴ溯��ִ�н��
    CPU_SR_ALLOC(); 
    
    OSTmrQty--;                 //��ʱ��������1

    CPU_CRITICAL_ENTER();         //�����ٽ��
    switch (p_tmr->State) 
    {                      
        case OS_TMR_STATE_RUNNING:                //�����ʱ����������
             OS_TmrUnlink(p_tmr);                 //�ӵ�ǰ��ʱ�����б��Ƴ���ʱ��  
             OS_TmrClr(p_tmr);                    //��λ��ʱ����ָ��
            *p_err   = OS_ERR_NONE;               //��������Ϊ���޴���
             success = 1;                  
             break;

        case OS_TMR_STATE_STOPPED:                //�����ʱ���ѱ�ֹͣ  
        case OS_TMR_STATE_COMPLETED:              //�����ʱ������ɵ�һ�ζ�ʱ
             OS_TmrClr(p_tmr);                    //��λ��ʱ����ָ��
            *p_err   = OS_ERR_NONE;               //��������Ϊ���޴���
             success = 1;                  
             break;
             
        case OS_TMR_STATE_UNUSED:                 //�����ʱ���ѱ�ɾ��
            *p_err   = OS_ERR_TMR_INACTIVE;       //��������Ϊ����ʱ��δ���
             success = 0;                 
             break;

        default:                                  //�����ʱ����״̬����Ԥ��
            *p_err   = OS_ERR_TMR_INVALID_STATE;  //��������Ϊ����ʱ����Ч��
             success = 0;                 
             break;
    }
    CPU_CRITICAL_EXIT(); 
    return (success);                             //����ִ�н��
}


CPU_BOOLEAN  OSTmrStart (OS_TMR  *p_tmr,  //��ʱ�����ƿ�ָ��
                         OS_ERR  *p_err)  //���ش�������
{
    CPU_BOOLEAN  success; //�ݴ溯��ִ�н��
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();         //�����ٽ��
    
    switch (p_tmr->State) 
    {                     
        case OS_TMR_STATE_RUNNING:                //�����ʱ���������У�������
             OS_TmrUnlink(p_tmr);                 //�Ӷ�ʱ�������Ƴ��ö�ʱ��
             OS_TmrLink(p_tmr, OS_OPT_LINK_DLY);  //���ö�ʱ�����²��뵽��ʱ����
            *p_err   = OS_ERR_NONE;               //��������Ϊ���޴���
             success = 1;             
             break;

        case OS_TMR_STATE_STOPPED:                //�����ʱ���ѱ�ֹͣ������  
        case OS_TMR_STATE_COMPLETED:              //�����ʱ��������ˣ�����  
             OS_TmrLink(p_tmr, OS_OPT_LINK_DLY);  //���ö�ʱ�����²��뵽��ʱ����
            *p_err   = OS_ERR_NONE;               //��������Ϊ���޴���
             success = 1;      
             break;

        case OS_TMR_STATE_UNUSED:                 //�����ʱ��δ������
            *p_err   = OS_ERR_TMR_INACTIVE;       //��������Ϊ����ʱ��δ���
             success = 0;              
             break;

        default:                                  //�����ʱ����״̬����Ԥ��
            *p_err = OS_ERR_TMR_INVALID_STATE;    //��������Ϊ����ʱ����Ч��
             success = 0;               
             break;
    }
    CPU_CRITICAL_EXIT(); 
    
    return (success);                             //����ִ�н��
}



CPU_BOOLEAN  OSTmrStop (OS_TMR  *p_tmr,          //��ʱ�����ƿ�ָ��
                        OS_OPT   opt,            //ѡ��
                        void    *p_callback_arg, //�����ص��������²���
                        OS_ERR  *p_err)          //���ش�������
{
    
    CPU_BOOLEAN          success;  //�ݴ溯��ִ�н��
    
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();         //�����ٽ��
    
    switch (p_tmr->State) 
    {                                              
        case OS_TMR_STATE_RUNNING:              //�����ʱ����������
             OS_TmrUnlink(p_tmr);               //�Ӷ�ʱ�����б����Ƴ��ö�ʱ��
            *p_err = OS_ERR_NONE;               //��������Ϊ���޴���
             success = 1;
             break;

        case OS_TMR_STATE_COMPLETED:            //�����ʱ������ɵ�һ�ζ�ʱ                     
        case OS_TMR_STATE_STOPPED:              //�����ʱ���ѱ�ֹͣ                 
            *p_err   = OS_ERR_TMR_STOPPED;      //��������Ϊ����ʱ���ѱ�ֹͣ��
             success = 1;       
             break;

        case OS_TMR_STATE_UNUSED:               //����ö�ʱ��δ��������                    
            *p_err   = OS_ERR_TMR_INACTIVE;     //��������Ϊ����ʱ��δ���
             success = 0;           
             break;

        default:                                //�����ʱ��״̬����Ԥ��
            *p_err   = OS_ERR_TMR_INVALID_STATE;//��������Ϊ����ʱ��״̬�Ƿ���
             success = 0;             
             break;
    }
    CPU_CRITICAL_EXIT(); 
    
    return (success);                           //����ִ�н��
} 


void  OS_TmrClr (OS_TMR  *p_tmr)
{
    p_tmr->State          = OS_TMR_STATE_UNUSED;            /* Clear timer fields                                     */
    p_tmr->Type           = 0;
    p_tmr->NamePtr        = (CPU_CHAR          *)((void *)"?TMR");
    p_tmr->Dly            = (OS_TICK            )0;
    p_tmr->Match          = (OS_TICK            )0;
    p_tmr->Remain         = (OS_TICK            )0;
    p_tmr->Period         = (OS_TICK            )0;
    p_tmr->Opt            = (OS_OPT             )0;
    p_tmr->CallbackPtr    = (OS_TMR_CALLBACK_PTR)0;
    p_tmr->CallbackPtrArg = (void              *)0;
    p_tmr->NextPtr        = (OS_TMR            *)0;
    p_tmr->PrevPtr        = (OS_TMR            *)0;
}



void  OS_TmrInit (OS_ERR  *p_err)
{
    CPU_CHAR   i;
    OS_TMR_SPOKE     *p_spoke;

    OSTmrUpdateCnt = 10;

    OSTmrUpdateCtr   = OSTmrUpdateCnt;

    OSTmrTickCtr     = (OS_TICK)0;

    OSTmrTaskTimeMax = (CPU_TS)0;
    
    for (i = 0u; i < OSCfg_TmrWheelSize; i++) 
    {
        p_spoke                = &OSCfg_TmrWheel[i];
        p_spoke->NbrEntries    = (OS_OBJ_QTY)0;
        p_spoke->NbrEntriesMax = (OS_OBJ_QTY)0;
        p_spoke->FirstPtr      = (OS_TMR   *)0;
    }
}




void  OS_TmrLink (OS_TMR  *p_tmr, OS_OPT   opt)
{
    OS_TMR_SPOKE     *p_spoke;
    OS_TMR           *p_tmr0;
    OS_TMR           *p_tmr1;
    OS_TMR_SPOKE_IX   spoke;



    p_tmr->State = OS_TMR_STATE_RUNNING;                           //���ö�ʱ��Ϊ����״̬
    if (opt == OS_OPT_LINK_PERIODIC) 
    {                             //�����ʱ�����ٴβ���
        p_tmr->Match = p_tmr->Period + OSTmrTickCtr;               //ƥ��ʱ��Ӹ���������ֵ
    } 
    else 
    {                                                              //�����ʱ�����״β���
        if (p_tmr->Dly == (OS_TICK)0) 
        {                                                          //�����ʱ���� Dly = 0
            p_tmr->Match = p_tmr->Period + OSTmrTickCtr;           //ƥ��ʱ��Ӹ���������ֵ
        } 
        else
        {                                                          //�����ʱ���� Dly != 0
            p_tmr->Match = p_tmr->Dly    + OSTmrTickCtr;           //ƥ��ʱ��Ӹ� Dly
        }
    }
    
    spoke  = (OS_TMR_SPOKE_IX)(p_tmr->Match % OSCfg_TmrWheelSize); //ͨ����ϣ�㷨���ý��ö�ʱ��
    p_spoke = &OSCfg_TmrWheel[spoke];                              //���뵽��ʱ���ֵ��ĸ��б�

    if (p_spoke->FirstPtr ==  (OS_TMR *)0) 
    {                
        p_tmr->NextPtr      = (OS_TMR *)0;                  //ֱ�ӽ��ö�ʱ����Ϊ�б�ĵ�һ��Ԫ�ء�
        p_tmr->PrevPtr      = (OS_TMR *)0;
        p_spoke->FirstPtr   = p_tmr;
        p_spoke->NbrEntries = 1u;
    } 
    else //����б�ǿ�
    {                                                
        p_tmr->Remain  = p_tmr->Match - OSTmrTickCtr;       //�����ʱ�� p_tmr ��ʣ��ʱ��
                       
        p_tmr1         = p_spoke->FirstPtr;                 //ȡ�б���׸�Ԫ�ص� p_tmr1
        while (p_tmr1 != (OS_TMR *)0) 
        {                    
            p_tmr1->Remain = p_tmr1->Match- OSTmrTickCtr;                 
                           
            if (p_tmr->Remain > p_tmr1->Remain)             //��� p_tmr ��ʣ��ʱ����� p_tmr1 ��
            {          
                if (p_tmr1->NextPtr  != (OS_TMR *)0)        //��� p_tmr1 ����ǿ�
                {      
                    p_tmr1            = p_tmr1->NextPtr;    //ȡp_tmr1��һ����ʱ��Ϊ�µ�p_tmr1������һ��ѭ��
                } 
                else                                        //��� p_tmr1 ����Ϊ��
                {                                    
                    p_tmr->NextPtr    = (OS_TMR *)0;        //�� p_tmr �嵽 p_tmr1 �ĺ��棬����ѭ��
                    p_tmr->PrevPtr    =  p_tmr1;
                    p_tmr1->NextPtr   =  p_tmr;             
                    p_tmr1            = (OS_TMR *)0;        
                }
            } 
            else                                            //��� p_tmr ��ʣ��ʱ�䲻���� p_tmr1 �ģ�
            {                                        
                if (p_tmr1->PrevPtr == (OS_TMR *)0)         //�� p_tmr ���뵽 p_tmr1 ��ǰһ��������ѭ����
                {       
                    p_tmr->PrevPtr    = (OS_TMR *)0;
                    p_tmr->NextPtr    = p_tmr1;
                    p_tmr1->PrevPtr   = p_tmr;
                    p_spoke->FirstPtr = p_tmr;
                }
                else 
                {                                   
                    p_tmr0            = p_tmr1->PrevPtr;
                    p_tmr->PrevPtr    = p_tmr0;
                    p_tmr->NextPtr    = p_tmr1;
                    p_tmr0->NextPtr   = p_tmr;
                    p_tmr1->PrevPtr   = p_tmr;
                }
                
                p_tmr1 = (OS_TMR *)0;                      
            }
        }
        
        p_spoke->NbrEntries++;                              //�б�Ԫ�س�Ա����1
        
    }
    
    if (p_spoke->NbrEntriesMax < p_spoke->NbrEntries)       //�����б��Ա�����ֵ��ʷ��¼
    {     
        p_spoke->NbrEntriesMax = p_spoke->NbrEntries;
    }
}


void  OS_TmrUnlink (OS_TMR  *p_tmr)   //��ʱ�����ƿ�ָ��
{
    OS_TMR_SPOKE    *p_spoke;
    OS_TMR          *p_tmr1;
    OS_TMR          *p_tmr2;
    OS_TMR_SPOKE_IX  spoke;



    spoke   = (OS_TMR_SPOKE_IX)(p_tmr->Match % OSCfg_TmrWheelSize); //�����ʱһ����ͨ����ϣ�㷨�ҳ�
    p_spoke = &OSCfg_TmrWheel[spoke];                               //�ö�ʱ���ڶ�ʱ���ֵ��ĸ��б�

    if (p_spoke->FirstPtr == p_tmr)                                 //��� p_tmr ���б���׸�Ԫ��
    {                       
        p_tmr1            = (OS_TMR *)p_tmr->NextPtr;       //ȡ p_tmr ��һ��Ԫ��Ϊ p_tmr1(����Ϊ��)
        p_spoke->FirstPtr = (OS_TMR *)p_tmr1;               //���׸�Ϊ p_tmr1 
        if (p_tmr1 != (OS_TMR *)0)                          //��� p_tmr1 ȷ���ǿ�
        {                        
            p_tmr1->PrevPtr = (OS_TMR *)0;                  //p_tmr1 ��ǰ�����
        }
    } 
    else                                                    //��� p_tmr �����б���׸�Ԫ��
    {                                                
        p_tmr1          = (OS_TMR *)p_tmr->PrevPtr;         //�� p_tmr ���б��Ƴ������� p_tmr 
        p_tmr2          = (OS_TMR *)p_tmr->NextPtr;         //ǰ�������Ԫ��������һ��.
        p_tmr1->NextPtr = p_tmr2;
        if (p_tmr2 != (OS_TMR *)0) 
        {                        
            p_tmr2->PrevPtr = (OS_TMR *)p_tmr1;
        }
    }
    
    p_tmr->State   = OS_TMR_STATE_STOPPED;   //��λ p_tmr ��ָ��             
    p_tmr->NextPtr = (OS_TMR *)0;
    p_tmr->PrevPtr = (OS_TMR *)0;
    p_spoke->NbrEntries--;                   //�б�Ԫ�س�Ա��1
}


void  OS_TmrTask (void)
{
    CPU_BOOLEAN          done;
    OS_TMR_CALLBACK_PTR  p_fnct;
    OS_TMR_SPOKE        *p_spoke;
    OS_TMR              *p_tmr;
    OS_TMR              *p_tmr_next;
    OS_TMR_SPOKE_IX      spoke;
    
    OSTmrTickCtr++;                                          
    
    spoke    = (OS_TMR_SPOKE_IX)(OSTmrTickCtr % OSCfg_TmrWheelSize);
    p_spoke  = &OSCfg_TmrWheel[spoke];
    p_tmr    = p_spoke->FirstPtr;
    done     = 0;
    
    while (done == 0) 
    {
        if (p_tmr != (OS_TMR *)0) 
        {
            p_tmr_next = (OS_TMR *)p_tmr->NextPtr;           /* Point to next tmr to update because current ...   */
                                                             /* ... timer could get unlinked from the wheel.      */
            if (OSTmrTickCtr == p_tmr->Match) 
            {            
                OS_TmrUnlink(p_tmr); 
                
                if (p_tmr->Opt == OS_OPT_TMR_PERIODIC) 
                {
                    OS_TmrLink(p_tmr,OS_OPT_LINK_PERIODIC);  
                } 
                else
                {
                    p_tmr->State = OS_TMR_STATE_COMPLETED;   /* Indicate that the timer has completed             */
                }
                
                p_fnct = p_tmr->CallbackPtr;                 /* Execute callback function if available            */
                if (p_fnct != (OS_TMR_CALLBACK_PTR)0) 
                {
                    (*p_fnct)((void *)p_tmr,p_tmr->CallbackPtrArg);
                }
                
                p_tmr = p_tmr_next;                          /* See if next timer matches                         */
            } 
            else 
            {
                done  = 1;
            }
        } 
        else 
        {
            done = 1;
        }
    }
}


