#include "os.h"



void  OS_SemClr (OS_SEM  *p_sem)
{
    p_sem->Type    = 0;                                     /* Mark the data structure as a NONE                      */
    p_sem->Ctr     = (OS_SEM_CTR)0;                         /* Set semaphore value                                    */
    p_sem->TS      = (CPU_TS    )0;                         /* Clear the time stamp                                   */
    p_sem->NamePtr = (CPU_CHAR *)((void *)"?SEM");
    OS_PendListInit(&p_sem->PendList);                      /* Initialize the waiting list                            */
}




void  OSSemCreate (OS_SEM      *p_sem,  //多值信号量控制块指针
                   CPU_CHAR    *p_name, //多值信号量名称
                   OS_SEM_CTR   cnt,    //资源数目或事件是否发生标志
                   OS_ERR      *p_err)  //返回错误类型
{
    CPU_SR_ALLOC(); //使用到临界段（在关/开中断时）时必需该宏，该宏声明和定义一个局部变
                    //量，用于保存关中断前的 CPU 状态寄存器 SR（临界段关中断只需保存SR）
                    //，开中断时将该值还原。 

    CPU_CRITICAL_ENTER();               //进入临界段
    p_sem->Type    = 0;  
    p_sem->Ctr     = cnt;                                 
    p_sem->TS      = (CPU_TS)0;
    p_sem->NamePtr = p_name;                               
    OS_PendListInit(&p_sem->PendList);   //初始化该多值信号量的等待列表     

    OSSemQty++;                         //多值信号量个数加1

    CPU_CRITICAL_EXIT();                //退出临界段（无调度）
   *p_err = OS_ERR_NONE;                 //错误类型为“无错误”
}



OS_OBJ_QTY  OSSemDel (OS_SEM  *p_sem,  //多值信号量指针
                      OS_OPT   opt,    //选项
                      OS_ERR  *p_err)  //返回错误类型
{
    OS_OBJ_QTY     nbr_tasks;
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_tcb;
    CPU_TS         ts;
    
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();                      //关中断
    p_pend_list = &p_sem->PendList;            //获取信号量的等待列表到 p_pend_list
    nbr_tasks = 0;
    switch (opt) 
    {                           
        case OS_OPT_DEL_NO_PEND:               //如果只在没有任务等待的情况下删除信号量
        {
             if (p_pend_list->HeadPtr == 0)
             {
                 OSSemQty--;                   //信号量数目减1
                 OS_SemClr(p_sem);             //清除信号量内容
                 CPU_CRITICAL_EXIT();          //开中断
                *p_err = OS_ERR_NONE;          //返回错误类型为“无错误”
             } 
             else 
             {                         
                 CPU_CRITICAL_EXIT();          //开中断
                *p_err = OS_ERR_TASK_WAITING;  //返回错误类型为“有任务在等待该信号量”
             }
             break;
         }

        case OS_OPT_DEL_ALWAYS:                             //如果必须删除信号量
        {
            
             while (p_pend_list->HeadPtr != 0)
             {                                              //逐个移除该信号量等待列表中的任务
                 p_tcb       = p_pend_list->HeadPtr;
                 OS_PendObjDel((OS_PEND_OBJ *)((void *)p_sem),p_tcb,ts);
                 nbr_tasks++;
             }
             
             OSSemQty--;                                    //信号量数目减1
             OS_SemClr(p_sem);                              //清除信号量内容
             CPU_CRITICAL_EXIT();                           //减锁调度器，但不进行调度
             OSSched();                                     //任务调度，执行最高优先级的就绪任务
            *p_err = OS_ERR_NONE;                           //返回错误类型为“无错误”
             break;
        }
        default: 
        {
             CPU_CRITICAL_EXIT();                           //开中断
            *p_err = OS_ERR_OPT_INVALID;                    //返回错误类型为“选项非法”
             break;
        }
    }
    return ((OS_OBJ_QTY)nbr_tasks);                         //返回删除信号量前等待其的任务数
}




OS_SEM_CTR  OSSemPend (OS_SEM   *p_sem,   //多值信号量指针
                       OS_TICK   timeout, //等待超时时间
                       OS_OPT    opt,     //选项
                       CPU_TS   *p_ts,    //等到信号量时的时间戳
                       OS_ERR   *p_err)   //返回错误类型
{
    OS_SEM_CTR    ctr;
    CPU_SR_ALLOC();
    
    CPU_CRITICAL_ENTER();                     //关中断
    
    if (p_sem->Ctr > (OS_SEM_CTR)0) 
    {       
        p_sem->Ctr--;                         //资源数目减1
        ctr   = p_sem->Ctr;                   //获取信号量的当前资源数目
        CPU_CRITICAL_EXIT();                  //开中断
       *p_err = OS_ERR_NONE;                  //返回错误类型为“无错误”
        return (ctr);                         //返回信号量的当前资源数目，不继续执行
    }
    
    if ((opt & OS_OPT_PEND_NON_BLOCKING) != (OS_OPT)0) 
    { 
        ctr   = p_sem->Ctr;                                 //获取信号量的资源数目到 ctr
        CPU_CRITICAL_EXIT();                                //开中断
       *p_err = OS_ERR_PEND_WOULD_BLOCK;                    //返回错误类型为“等待渴求堵塞”  
        return (ctr);                                       //返回信号量的当前资源数目，不继续执行
    } 
                                                   
    OS_Pend((OS_PEND_OBJ *)((void *)p_sem), OS_TASK_PEND_ON_SEM,timeout);

    CPU_CRITICAL_EXIT();            

    OSSched();                                              //找到并调度最高优先级就绪任务

    CPU_CRITICAL_ENTER();
    ctr = p_sem->Ctr;                                       //获取信号量的当前资源数目
    CPU_CRITICAL_EXIT();                                    //开中断
    return (ctr);                                           //返回信号量的当前资源数目
}



void  OS_SemInit (OS_ERR  *p_err)
{
    OSSemQty        = (OS_OBJ_QTY)0;
    *p_err          = OS_ERR_NONE;
}



OS_SEM_CTR  OS_SemPost (OS_SEM  *p_sem, //多值信号量指针
                        OS_OPT   opt,   //选项
                        CPU_TS   ts,    //时间戳
                        OS_ERR  *p_err) //返回错误类型
{
    OS_OBJ_QTY     cnt;
    OS_SEM_CTR     ctr;
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_tcb;
    OS_TCB        *p_tcb_next;
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();                                   //关中断
    
    p_pend_list = &p_sem->PendList;                         //取出该信号量的等待列表
    
    if (p_pend_list->HeadPtr == 0) 
    {                      
                          
        if ((int)(p_sem->Ctr) == -1) 
        {  
            CPU_CRITICAL_EXIT();        
            *p_err = OS_ERR_SEM_OVF;
            return ((OS_SEM_CTR)0);
        }

        p_sem->Ctr++;                                       //信号量计数值不溢出则加1
        ctr       = p_sem->Ctr;                             //获取信号量计数值到 ctr
        p_sem->TS = ts;                                     //保存时间戳
        CPU_CRITICAL_EXIT();                                //则开中断
       *p_err     = OS_ERR_NONE;                            //返回错误类型为“无错误”
        return (ctr);                                       //返回信号量的计数值，不继续执行
    }

    if ((opt & OS_OPT_POST_ALL) != (OS_OPT)0) 
    {                                                       //如果要将信号量发布给所有等待任务
        cnt = p_pend_list->NbrEntries;                      //获取等待任务数目到 cnt
    } 
    else 
    {                                                       //如果要将信号量发布给优先级最高的等待任务
        cnt = (OS_OBJ_QTY)1;                                //将要操作的任务数为1，cnt 置1
    }
    
    p_tcb = p_pend_list->HeadPtr;                           //等待链表中的任务都是按优先级插入的 所以第一个是等待中优先级最高的
    
    while (cnt > 0u) 
    {                                                    
        p_tcb_next = p_tcb->PendNextPtr;            
        
        OS_Post((OS_PEND_OBJ *)((void *)p_sem),             //发布信号量给当前任务
                p_tcb,
                (void      *)0,
                (OS_MSG_SIZE)0,
                ts);
        p_tcb = p_tcb_next;       
        cnt--;
    }
    
    ctr = p_sem->Ctr;                                       //获取信号量计数值到 ctr
    
    CPU_CRITICAL_EXIT();                                    //减锁调度器，但不执行任务调度
    
    if ((opt & OS_OPT_POST_NO_SCHED) == (OS_OPT)0)          //如果 opt 没选择“发布时不调度任务”
    {       
        OSSched();                                          //任务调度
    }
    
    *p_err = OS_ERR_NONE;                                   //返回错误类型为“无错误”
    return (ctr);                                           //返回信号量的当前计数值
}


OS_SEM_CTR  OSSemPost (OS_SEM  *p_sem,    //多值信号量控制块指针
                       OS_OPT   opt,      //选项
                       OS_ERR  *p_err)    //返回错误类型
{
    OS_SEM_CTR  ctr;
    CPU_TS      ts;

    ctr = OS_SemPost(p_sem,opt,ts,p_err);

    return (ctr);                                 //返回信号的当前计数值
}
