#include <os.h>


static void  OS_TmrUnlink (OS_TMR  *p_tmr);
static void  OS_TmrLink (OS_TMR  *p_tmr, OS_OPT   opt);
static void  OS_TmrClr (OS_TMR  *p_tmr);

#define  OS_OPT_LINK_DLY        (OS_OPT)(0u)
#define  OS_OPT_LINK_PERIODIC   (OS_OPT)(1u)


void  OSTmrCreate (OS_TMR               *p_tmr,          //定时器控制块指针
                   CPU_CHAR             *p_name,         //命名定时器，有助于调试
                   OS_TICK               dly,            //初始定时节拍数
                   OS_TICK               period,         //周期定时重载节拍数
                   OS_OPT                opt,            //选项
                   OS_TMR_CALLBACK_PTR   p_callback,     //定时到期时的回调函数
                   void                 *p_callback_arg, //传给回调函数的参数
                   OS_ERR               *p_err)          //返回错误类型
{
    CPU_SR_ALLOC(); //使用到临界段（在关/开中断时）时必需该宏，该宏声明和定义一个局部变
                    //量，用于保存关中断前的 CPU 状态寄存器 SR（临界段关中断只需保存SR）
                    //，开中断时将该值还原。 

    CPU_CRITICAL_ENTER();         //进入临界段
    
    p_tmr->State          = (OS_STATE           )OS_TMR_STATE_STOPPED;  //初始化定时器指标
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


CPU_BOOLEAN  OSTmrDel (OS_TMR  *p_tmr, //定时器控制块指针
                       OS_ERR  *p_err) //返回错误类型
{
    CPU_BOOLEAN  success;  //暂存函数执行结果
    CPU_SR_ALLOC(); 
    
    OSTmrQty--;                 //定时器个数减1

    CPU_CRITICAL_ENTER();         //进入临界段
    switch (p_tmr->State) 
    {                      
        case OS_TMR_STATE_RUNNING:                //如果定时器正在运行
             OS_TmrUnlink(p_tmr);                 //从当前定时器轮列表移除定时器  
             OS_TmrClr(p_tmr);                    //复位定时器的指标
            *p_err   = OS_ERR_NONE;               //错误类型为“无错误”
             success = 1;                  
             break;

        case OS_TMR_STATE_STOPPED:                //如果定时器已被停止  
        case OS_TMR_STATE_COMPLETED:              //如果定时器已完成第一次定时
             OS_TmrClr(p_tmr);                    //复位定时器的指标
            *p_err   = OS_ERR_NONE;               //错误类型为“无错误”
             success = 1;                  
             break;
             
        case OS_TMR_STATE_UNUSED:                 //如果定时器已被删除
            *p_err   = OS_ERR_TMR_INACTIVE;       //错误类型为“定时器未激活”
             success = 0;                 
             break;

        default:                                  //如果定时器的状态超出预期
            *p_err   = OS_ERR_TMR_INVALID_STATE;  //错误类型为“定时器无效”
             success = 0;                 
             break;
    }
    CPU_CRITICAL_EXIT(); 
    return (success);                             //返回执行结果
}


CPU_BOOLEAN  OSTmrStart (OS_TMR  *p_tmr,  //定时器控制块指针
                         OS_ERR  *p_err)  //返回错误类型
{
    CPU_BOOLEAN  success; //暂存函数执行结果
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();         //进入临界段
    
    switch (p_tmr->State) 
    {                     
        case OS_TMR_STATE_RUNNING:                //如果定时器正在运行，则重启
             OS_TmrUnlink(p_tmr);                 //从定时器轮里移除该定时器
             OS_TmrLink(p_tmr, OS_OPT_LINK_DLY);  //将该定时器重新插入到定时器轮
            *p_err   = OS_ERR_NONE;               //错误类型为“无错误”
             success = 1;             
             break;

        case OS_TMR_STATE_STOPPED:                //如果定时器已被停止，则开启  
        case OS_TMR_STATE_COMPLETED:              //如果定时器已完成了，则开启  
             OS_TmrLink(p_tmr, OS_OPT_LINK_DLY);  //将该定时器重新插入到定时器轮
            *p_err   = OS_ERR_NONE;               //错误类型为“无错误”
             success = 1;      
             break;

        case OS_TMR_STATE_UNUSED:                 //如果定时器未被创建
            *p_err   = OS_ERR_TMR_INACTIVE;       //错误类型为“定时器未激活”
             success = 0;              
             break;

        default:                                  //如果定时器的状态超出预期
            *p_err = OS_ERR_TMR_INVALID_STATE;    //错误类型为“定时器无效”
             success = 0;               
             break;
    }
    CPU_CRITICAL_EXIT(); 
    
    return (success);                             //返回执行结果
}



CPU_BOOLEAN  OSTmrStop (OS_TMR  *p_tmr,          //定时器控制块指针
                        OS_OPT   opt,            //选项
                        void    *p_callback_arg, //传给回调函数的新参数
                        OS_ERR  *p_err)          //返回错误类型
{
    
    CPU_BOOLEAN          success;  //暂存函数执行结果
    
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();         //进入临界段
    
    switch (p_tmr->State) 
    {                                              
        case OS_TMR_STATE_RUNNING:              //如果定时器正在运行
             OS_TmrUnlink(p_tmr);               //从定时器轮列表里移除该定时器
            *p_err = OS_ERR_NONE;               //错误类型为“无错误”
             success = 1;
             break;

        case OS_TMR_STATE_COMPLETED:            //如果定时器已完成第一次定时                     
        case OS_TMR_STATE_STOPPED:              //如果定时器已被停止                 
            *p_err   = OS_ERR_TMR_STOPPED;      //错误类型为“定时器已被停止”
             success = 1;       
             break;

        case OS_TMR_STATE_UNUSED:               //如果该定时器未被创建过                    
            *p_err   = OS_ERR_TMR_INACTIVE;     //错误类型为“定时器未激活”
             success = 0;           
             break;

        default:                                //如果定时器状态超出预期
            *p_err   = OS_ERR_TMR_INVALID_STATE;//错误类型为“定时器状态非法”
             success = 0;             
             break;
    }
    CPU_CRITICAL_EXIT(); 
    
    return (success);                           //返回执行结果
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



    p_tmr->State = OS_TMR_STATE_RUNNING;                           //重置定时器为运行状态
    if (opt == OS_OPT_LINK_PERIODIC) 
    {                             //如果定时器是再次插入
        p_tmr->Match = p_tmr->Period + OSTmrTickCtr;               //匹配时间加个周期重载值
    } 
    else 
    {                                                              //如果定时器是首次插入
        if (p_tmr->Dly == (OS_TICK)0) 
        {                                                          //如果定时器的 Dly = 0
            p_tmr->Match = p_tmr->Period + OSTmrTickCtr;           //匹配时间加个周期重载值
        } 
        else
        {                                                          //如果定时器的 Dly != 0
            p_tmr->Match = p_tmr->Dly    + OSTmrTickCtr;           //匹配时间加个 Dly
        }
    }
    
    spoke  = (OS_TMR_SPOKE_IX)(p_tmr->Match % OSCfg_TmrWheelSize); //通过哈希算法觉得将该定时器
    p_spoke = &OSCfg_TmrWheel[spoke];                              //插入到定时器轮的哪个列表。

    if (p_spoke->FirstPtr ==  (OS_TMR *)0) 
    {                
        p_tmr->NextPtr      = (OS_TMR *)0;                  //直接将该定时器作为列表的第一个元素。
        p_tmr->PrevPtr      = (OS_TMR *)0;
        p_spoke->FirstPtr   = p_tmr;
        p_spoke->NbrEntries = 1u;
    } 
    else //如果列表非空
    {                                                
        p_tmr->Remain  = p_tmr->Match - OSTmrTickCtr;       //算出定时器 p_tmr 的剩余时间
                       
        p_tmr1         = p_spoke->FirstPtr;                 //取列表的首个元素到 p_tmr1
        while (p_tmr1 != (OS_TMR *)0) 
        {                    
            p_tmr1->Remain = p_tmr1->Match- OSTmrTickCtr;                 
                           
            if (p_tmr->Remain > p_tmr1->Remain)             //如果 p_tmr 的剩余时间大于 p_tmr1 的
            {          
                if (p_tmr1->NextPtr  != (OS_TMR *)0)        //如果 p_tmr1 后面非空
                {      
                    p_tmr1            = p_tmr1->NextPtr;    //取p_tmr1后一个定时器为新的p_tmr1进行下一次循环
                } 
                else                                        //如果 p_tmr1 后面为空
                {                                    
                    p_tmr->NextPtr    = (OS_TMR *)0;        //将 p_tmr 插到 p_tmr1 的后面，结束循环
                    p_tmr->PrevPtr    =  p_tmr1;
                    p_tmr1->NextPtr   =  p_tmr;             
                    p_tmr1            = (OS_TMR *)0;        
                }
            } 
            else                                            //如果 p_tmr 的剩余时间不大于 p_tmr1 的，
            {                                        
                if (p_tmr1->PrevPtr == (OS_TMR *)0)         //将 p_tmr 插入到 p_tmr1 的前一个，结束循环。
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
        
        p_spoke->NbrEntries++;                              //列表元素成员数加1
        
    }
    
    if (p_spoke->NbrEntriesMax < p_spoke->NbrEntries)       //更新列表成员数最大值历史记录
    {     
        p_spoke->NbrEntriesMax = p_spoke->NbrEntries;
    }
}


void  OS_TmrUnlink (OS_TMR  *p_tmr)   //定时器控制块指针
{
    OS_TMR_SPOKE    *p_spoke;
    OS_TMR          *p_tmr1;
    OS_TMR          *p_tmr2;
    OS_TMR_SPOKE_IX  spoke;



    spoke   = (OS_TMR_SPOKE_IX)(p_tmr->Match % OSCfg_TmrWheelSize); //与插入时一样，通过哈希算法找出
    p_spoke = &OSCfg_TmrWheel[spoke];                               //该定时器在定时器轮的哪个列表。

    if (p_spoke->FirstPtr == p_tmr)                                 //如果 p_tmr 是列表的首个元素
    {                       
        p_tmr1            = (OS_TMR *)p_tmr->NextPtr;       //取 p_tmr 后一个元素为 p_tmr1(可能为空)
        p_spoke->FirstPtr = (OS_TMR *)p_tmr1;               //表首改为 p_tmr1 
        if (p_tmr1 != (OS_TMR *)0)                          //如果 p_tmr1 确定非空
        {                        
            p_tmr1->PrevPtr = (OS_TMR *)0;                  //p_tmr1 的前面清空
        }
    } 
    else                                                    //如果 p_tmr 不是列表的首个元素
    {                                                
        p_tmr1          = (OS_TMR *)p_tmr->PrevPtr;         //将 p_tmr 从列表移除，并将 p_tmr 
        p_tmr2          = (OS_TMR *)p_tmr->NextPtr;         //前后的两个元素连接在一起.
        p_tmr1->NextPtr = p_tmr2;
        if (p_tmr2 != (OS_TMR *)0) 
        {                        
            p_tmr2->PrevPtr = (OS_TMR *)p_tmr1;
        }
    }
    
    p_tmr->State   = OS_TMR_STATE_STOPPED;   //复位 p_tmr 的指标             
    p_tmr->NextPtr = (OS_TMR *)0;
    p_tmr->PrevPtr = (OS_TMR *)0;
    p_spoke->NbrEntries--;                   //列表元素成员减1
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


