#ifndef   OS_H
#define   OS_H


//头文件顺序很重要哦
#include "os_cfg.h"
#include "os_cfg_app.h"
#include "cpu_core.h"
#include "os_type.h"
#include "os_cpu.h"
#include "cpu.h"


#ifdef   OS_GLOBALS 
    #define OS_EXT  
#else 
    #define OS_EXT extern
#endif
    
                                                                /* ------------------- TASK STATES ------------------ */
#define  OS_TASK_STATE_BIT_DLY               (OS_STATE)(0x01u)  /*   /-------- SUSPENDED bit                          */
                                                                /*   |                                                */
#define  OS_TASK_STATE_BIT_PEND              (OS_STATE)(0x02u)  /*   | /-----  PEND      bit                          */
                                                                /*   | |                                              */
#define  OS_TASK_STATE_BIT_SUSPENDED         (OS_STATE)(0x04u)  /*   | | /---  Delayed/Timeout bit                    */
                                                                /*   | | |                                            */
                                                                /*   V V V                                            */

#define  OS_TASK_STATE_RDY                    (OS_STATE)(  0u)  /*   0 0 0     Ready                                  */
#define  OS_TASK_STATE_DLY                    (OS_STATE)(  1u)  /*   0 0 1     Delayed or Timeout                     */
#define  OS_TASK_STATE_PEND                   (OS_STATE)(  2u)  /*   0 1 0     Pend                                   */
#define  OS_TASK_STATE_PEND_TIMEOUT           (OS_STATE)(  3u)  /*   0 1 1     Pend + Timeout                         */
#define  OS_TASK_STATE_SUSPENDED              (OS_STATE)(  4u)  /*   1 0 0     Suspended                              */
#define  OS_TASK_STATE_DLY_SUSPENDED          (OS_STATE)(  5u)  /*   1 0 1     Suspended + Delayed or Timeout         */
#define  OS_TASK_STATE_PEND_SUSPENDED         (OS_STATE)(  6u)  /*   1 1 0     Suspended + Pend                       */
#define  OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED (OS_STATE)(  7u)  /*   1 1 1     Suspended + Pend + Timeout             */
    

#define  OS_TASK_PEND_ON_NOTHING              (OS_STATE)(  0u)  /* Pending on nothing                                 */
#define  OS_TASK_PEND_ON_FLAG                 (OS_STATE)(  1u)  /* Pending on event flag group                        */
#define  OS_TASK_PEND_ON_TASK_Q               (OS_STATE)(  2u)  /* Pending on message to be sent to task              */
#define  OS_TASK_PEND_ON_MULTI                (OS_STATE)(  3u)  /* Pending on multiple semaphores and/or queues       */
#define  OS_TASK_PEND_ON_MUTEX                (OS_STATE)(  4u)  /* Pending on mutual exclusion semaphore              */
#define  OS_TASK_PEND_ON_Q                    (OS_STATE)(  5u)  /* Pending on queue                                   */
#define  OS_TASK_PEND_ON_SEM                  (OS_STATE)(  6u)  /* Pending on semaphore                               */
#define  OS_TASK_PEND_ON_TASK_SEM             (OS_STATE)(  7u)  /* Pending on signal  to be sent to task              */

/*
------------------------------------------------------------------------------------------------------------------------
*                                                    TASK PEND STATUS
*                                      (Status codes for OS_TCBs field .PendStatus)
------------------------------------------------------------------------------------------------------------------------
*/

#define  OS_STATUS_PEND_OK                   (OS_STATUS)(  0u)  /* Pending status OK, !pending, or pending complete   */
#define  OS_STATUS_PEND_ABORT                (OS_STATUS)(  1u)  /* Pending aborted                                    */
#define  OS_STATUS_PEND_DEL                  (OS_STATUS)(  2u)  /* Pending object deleted                             */
#define  OS_STATUS_PEND_TIMEOUT              (OS_STATUS)(  3u)  
   
typedef enum os_err 
{
    OS_ERR_NONE                      =     0u,

    OS_ERR_A                         = 10000u,
    OS_ERR_ACCEPT_ISR                = 10001u,

    OS_ERR_B                         = 11000u,

    OS_ERR_C                         = 12000u,
    OS_ERR_CREATE_ISR                = 12001u,

    OS_ERR_D                         = 13000u,
    OS_ERR_DEL_ISR                   = 13001u,

    OS_ERR_E                         = 14000u,

    OS_ERR_F                         = 15000u,
    OS_ERR_FATAL_RETURN              = 15001u,

    OS_ERR_FLAG_GRP_DEPLETED         = 15101u,
    OS_ERR_FLAG_NOT_RDY              = 15102u,
    OS_ERR_FLAG_PEND_OPT             = 15103u,
    OS_ERR_FLUSH_ISR                 = 15104u,

    OS_ERR_G                         = 16000u,

    OS_ERR_H                         = 17000u,

    OS_ERR_I                         = 18000u,
    OS_ERR_ILLEGAL_CREATE_RUN_TIME   = 18001u,

    OS_ERR_INT_Q                     = 18002u,                  /* OBSOLETE ERR CODE                                    */
    OS_ERR_INT_Q_FULL                = 18003u,                  /* OBSOLETE ERR CODE                                    */
    OS_ERR_INT_Q_SIZE                = 18004u,                  /* OBSOLETE ERR CODE                                    */
    OS_ERR_INT_Q_STK_INVALID         = 18005u,                  /* OBSOLETE ERR CODE                                    */
    OS_ERR_INT_Q_STK_SIZE_INVALID    = 18006u,                  /* OBSOLETE ERR CODE                                    */

    OS_ERR_ILLEGAL_DEL_RUN_TIME      = 18007u,

    OS_ERR_J                         = 19000u,

    OS_ERR_K                         = 20000u,

    OS_ERR_L                         = 21000u,
    OS_ERR_LOCK_NESTING_OVF          = 21001u,

    OS_ERR_M                         = 22000u,

    OS_ERR_MEM_CREATE_ISR            = 22201u,
    OS_ERR_MEM_FULL                  = 22202u,
    OS_ERR_MEM_INVALID_P_ADDR        = 22203u,
    OS_ERR_MEM_INVALID_BLKS          = 22204u,
    OS_ERR_MEM_INVALID_PART          = 22205u,
    OS_ERR_MEM_INVALID_P_BLK         = 22206u,
    OS_ERR_MEM_INVALID_P_MEM         = 22207u,
    OS_ERR_MEM_INVALID_P_DATA        = 22208u,
    OS_ERR_MEM_INVALID_SIZE          = 22209u,
    OS_ERR_MEM_NO_FREE_BLKS          = 22210u,

    OS_ERR_MSG_POOL_EMPTY            = 22301u,
    OS_ERR_MSG_POOL_NULL_PTR         = 22302u,

    OS_ERR_MUTEX_NOT_OWNER           = 22401u,
    OS_ERR_MUTEX_OWNER               = 22402u,
    OS_ERR_MUTEX_NESTING             = 22403u,
    OS_ERR_MUTEX_OVF                 = 22404u,

    OS_ERR_N                         = 23000u,
    OS_ERR_NAME                      = 23001u,
    OS_ERR_NO_MORE_ID_AVAIL          = 23002u,

    OS_ERR_O                         = 24000u,
    OS_ERR_OBJ_CREATED               = 24001u,
    OS_ERR_OBJ_DEL                   = 24002u,
    OS_ERR_OBJ_PTR_NULL              = 24003u,
    OS_ERR_OBJ_TYPE                  = 24004u,

    OS_ERR_OPT_INVALID               = 24101u,

    OS_ERR_OS_NOT_RUNNING            = 24201u,
    OS_ERR_OS_RUNNING                = 24202u,
    OS_ERR_OS_NOT_INIT               = 24203u,
    OS_ERR_OS_NO_APP_TASK            = 24204u,

    OS_ERR_P                         = 25000u,
    OS_ERR_PEND_ABORT                = 25001u,
    OS_ERR_PEND_ABORT_ISR            = 25002u,
    OS_ERR_PEND_ABORT_NONE           = 25003u,
    OS_ERR_PEND_ABORT_SELF           = 25004u,
    OS_ERR_PEND_DEL                  = 25005u,
    OS_ERR_PEND_ISR                  = 25006u,
    OS_ERR_PEND_LOCKED               = 25007u,
    OS_ERR_PEND_WOULD_BLOCK          = 25008u,

    OS_ERR_POST_NULL_PTR             = 25101u,
    OS_ERR_POST_ISR                  = 25102u,

    OS_ERR_PRIO_EXIST                = 25201u,
    OS_ERR_PRIO                      = 25202u,
    OS_ERR_PRIO_INVALID              = 25203u,

    OS_ERR_PTR_INVALID               = 25301u,

    OS_ERR_Q                         = 26000u,
    OS_ERR_Q_FULL                    = 26001u,
    OS_ERR_Q_EMPTY                   = 26002u,
    OS_ERR_Q_MAX                     = 26003u,
    OS_ERR_Q_SIZE                    = 26004u,

    OS_ERR_R                         = 27000u,
    OS_ERR_REG_ID_INVALID            = 27001u,
    OS_ERR_ROUND_ROBIN_1             = 27002u,
    OS_ERR_ROUND_ROBIN_DISABLED      = 27003u,

    OS_ERR_S                         = 28000u,
    OS_ERR_SCHED_INVALID_TIME_SLICE  = 28001u,
    OS_ERR_SCHED_LOCK_ISR            = 28002u,
    OS_ERR_SCHED_LOCKED              = 28003u,
    OS_ERR_SCHED_NOT_LOCKED          = 28004u,
    OS_ERR_SCHED_UNLOCK_ISR          = 28005u,

    OS_ERR_SEM_OVF                   = 28101u,
    OS_ERR_SET_ISR                   = 28102u,

    OS_ERR_STAT_RESET_ISR            = 28201u,
    OS_ERR_STAT_PRIO_INVALID         = 28202u,
    OS_ERR_STAT_STK_INVALID          = 28203u,
    OS_ERR_STAT_STK_SIZE_INVALID     = 28204u,
    OS_ERR_STATE_INVALID             = 28205u,
    OS_ERR_STATUS_INVALID            = 28206u,
    OS_ERR_STK_INVALID               = 28207u,
    OS_ERR_STK_SIZE_INVALID          = 28208u,
    OS_ERR_STK_LIMIT_INVALID         = 28209u,

    OS_ERR_T                         = 29000u,
    OS_ERR_TASK_CHANGE_PRIO_ISR      = 29001u,
    OS_ERR_TASK_CREATE_ISR           = 29002u,
    OS_ERR_TASK_DEL                  = 29003u,
    OS_ERR_TASK_DEL_IDLE             = 29004u,
    OS_ERR_TASK_DEL_INVALID          = 29005u,
    OS_ERR_TASK_DEL_ISR              = 29006u,
    OS_ERR_TASK_INVALID              = 29007u,
    OS_ERR_TASK_NO_MORE_TCB          = 29008u,
    OS_ERR_TASK_NOT_DLY              = 29009u,
    OS_ERR_TASK_NOT_EXIST            = 29010u,
    OS_ERR_TASK_NOT_SUSPENDED        = 29011u,
    OS_ERR_TASK_OPT                  = 29012u,
    OS_ERR_TASK_RESUME_ISR           = 29013u,
    OS_ERR_TASK_RESUME_PRIO          = 29014u,
    OS_ERR_TASK_RESUME_SELF          = 29015u,
    OS_ERR_TASK_RUNNING              = 29016u,
    OS_ERR_TASK_STK_CHK_ISR          = 29017u,
    OS_ERR_TASK_SUSPENDED            = 29018u,
    OS_ERR_TASK_SUSPEND_IDLE         = 29019u,
    OS_ERR_TASK_SUSPEND_INT_HANDLER  = 29020u,
    OS_ERR_TASK_SUSPEND_ISR          = 29021u,
    OS_ERR_TASK_SUSPEND_PRIO         = 29022u,
    OS_ERR_TASK_WAITING              = 29023u,
    OS_ERR_TASK_SUSPEND_CTR_OVF      = 29024u,

    OS_ERR_TCB_INVALID               = 29101u,

    OS_ERR_TLS_ID_INVALID            = 29120u,
    OS_ERR_TLS_ISR                   = 29121u,
    OS_ERR_TLS_NO_MORE_AVAIL         = 29122u,
    OS_ERR_TLS_NOT_EN                = 29123u,
    OS_ERR_TLS_DESTRUCT_ASSIGNED     = 29124u,

    OS_ERR_TICK_PRIO_INVALID         = 29201u,
    OS_ERR_TICK_STK_INVALID          = 29202u,
    OS_ERR_TICK_STK_SIZE_INVALID     = 29203u,
    OS_ERR_TICK_WHEEL_SIZE           = 29204u,

    OS_ERR_TIME_DLY_ISR              = 29301u,
    OS_ERR_TIME_DLY_RESUME_ISR       = 29302u,
    OS_ERR_TIME_GET_ISR              = 29303u,
    OS_ERR_TIME_INVALID_HOURS        = 29304u,
    OS_ERR_TIME_INVALID_MINUTES      = 29305u,
    OS_ERR_TIME_INVALID_SECONDS      = 29306u,
    OS_ERR_TIME_INVALID_MILLISECONDS = 29307u,
    OS_ERR_TIME_NOT_DLY              = 29308u,
    OS_ERR_TIME_SET_ISR              = 29309u,
    OS_ERR_TIME_ZERO_DLY             = 29310u,

    OS_ERR_TIMEOUT                   = 29401u,

    OS_ERR_TMR_INACTIVE              = 29501u,
    OS_ERR_TMR_INVALID_DEST          = 29502u,
    OS_ERR_TMR_INVALID_DLY           = 29503u,
    OS_ERR_TMR_INVALID_PERIOD        = 29504u,
    OS_ERR_TMR_INVALID_STATE         = 29505u,
    OS_ERR_TMR_INVALID               = 29506u,
    OS_ERR_TMR_ISR                   = 29507u,
    OS_ERR_TMR_NO_CALLBACK           = 29508u,
    OS_ERR_TMR_NON_AVAIL             = 29509u,
    OS_ERR_TMR_PRIO_INVALID          = 29510u,
    OS_ERR_TMR_STK_INVALID           = 29511u,
    OS_ERR_TMR_STK_SIZE_INVALID      = 29512u,
    OS_ERR_TMR_STOPPED               = 29513u,
    OS_ERR_TMR_INVALID_CALLBACK      = 29514u,

    OS_ERR_U                         = 30000u,

    OS_ERR_V                         = 31000u,

    OS_ERR_W                         = 32000u,

    OS_ERR_X                         = 33000u,

    OS_ERR_Y                         = 34000u,
    OS_ERR_YIELD_ISR                 = 34001u,

    OS_ERR_Z                         = 35000u
    
}OS_ERR;

typedef void                    (*OS_TMR_CALLBACK_PTR)(void *p_tmr, void *p_arg);
typedef struct os_tmr_spoke     OS_TMR_SPOKE;
typedef struct os_tmr           OS_TMR;

typedef struct os_rdy_list      OS_RDY_LIST;
typedef struct os_tick_spoke    OS_TICK_SPOKE;  
typedef struct os_tcb           OS_TCB;/* 任务控制块重定义 */ 
typedef struct os_pend_obj      OS_PEND_OBJ;
typedef struct os_pend_list     OS_PEND_LIST;

typedef struct os_sem           OS_SEM;
typedef struct os_mutex         OS_MUTEX;
typedef struct os_flag_grp      OS_FLAG_GRP;

struct  os_tmr
{
    OS_OBJ_TYPE          Type;
    CPU_CHAR            *NamePtr;                           /* Name to give the timer                                 */
    OS_TMR_CALLBACK_PTR  CallbackPtr;                       /* Function to call when timer expires                    */
    void                *CallbackPtrArg;                    /* Argument to pass to function when timer expires        */
    OS_TMR              *NextPtr;                           /* Double link list pointers                              */
    OS_TMR              *PrevPtr;
    OS_TICK              Match;                             /* Timer expires when OSTmrTickCtr matches this value     */
    OS_TICK              Remain;                            /* Amount of time remaining before timer expires          */
    OS_TICK              Dly;                               /* Delay before start of repeat                           */
    OS_TICK              Period;                            /* Period to repeat timer                                 */
    OS_OPT               Opt;                               /* Options (see OS_OPT_TMR_xxx)                           */
    OS_STATE             State;
};

 /* 任务控制块 数据类型声明 */ 
struct os_tcb 
{ 
    CPU_STK *StkPtr; 
    CPU_STK_SIZE StkSize;
    
    OS_TICK TaskDelayTicks;
    OS_PRIO Prio;
    OS_TCB *NextPtr;
    OS_TCB *PrevPtr;
    
    /*时基列表相关字段*/
    OS_TCB *TickNextPtr;
    OS_TCB *TickPrevPtr;
    OS_TICK_SPOKE *TickSpokePtr;
    
    OS_TICK TickCtrMatch;
    OS_TICK TickRemain;
    
    /* 时间片相关字段 */
    OS_TICK TimeQuanta;
    OS_TICK TimeQuantaCtr;
    
    OS_TCB        *PendNextPtr;                 
    OS_TCB        *PendPrevPtr;                      
    OS_PEND_OBJ   *PendObjPtr;                        
    OS_STATE      PendOn;                     
    OS_STATUS     PendStatus;      

    OS_PRIO       BasePrio;                          
    OS_MUTEX      *MutexGrpHeadPtr;//链接当前TCB拥有的互斥量

    OS_FLAGS             FlagsPend;/* Event flag(s) to wait on                               */
    OS_FLAGS             FlagsRdy; /* Event flags that made task ready to run                */
    OS_OPT               FlagsOpt; /* Options (See OS_OPT_FLAG_xxx)                          */
    
    
    OS_STATE TaskState;
#if OS_CFG_TASK_SUSPEND_EN > 0u
    /* 任务挂起函数OSTaskSuspend()计数器 */
    OS_NESTING_CTR SuspendCtr;
#endif
    
};

struct os_rdy_list 
{
    OS_TCB *HeadPtr;
    OS_TCB *TailPtr;
    OS_OBJ_QTY NbrEntries;
};

struct  os_pend_list
{
    OS_TCB              *HeadPtr;
    OS_TCB              *TailPtr;
    OS_OBJ_QTY          NbrEntries;
};

struct  os_pend_obj 
{
    OS_OBJ_TYPE          Type;
    CPU_CHAR            *NamePtr;
    OS_PEND_LIST         PendList;
};

struct  os_sem 
{                                                           /* ------------------ GENERIC  MEMBERS ------------------ */
    OS_OBJ_TYPE          Type;                              /* Should be set to OS_OBJ_TYPE_SEM                       */
    CPU_CHAR            *NamePtr;                           /* Pointer to Semaphore Name (NUL terminated ASCII)       */
    OS_PEND_LIST         PendList;   
    OS_SEM_CTR           Ctr;
    CPU_TS               TS;
};

struct  os_mutex 
{                                        
    OS_OBJ_TYPE          Type;                              /* Should be set to OS_OBJ_TYPE_MUTEX                     */
    CPU_CHAR            *NamePtr;                           /* Pointer to Mutex Name (NUL terminated ASCII)           */

    OS_PEND_LIST         PendList;                          /* List of tasks waiting on mutex                         */
                                                            /* ------------------ SPECIFIC MEMBERS ------------------ */
    OS_MUTEX            *MutexGrpNextPtr;
    OS_TCB              *OwnerTCBPtr;
    OS_NESTING_CTR       OwnerNestingCtr;                   /* Mutex is available when the counter is 0               */
    CPU_TS               TS;
};

struct  os_flag_grp 
{                                                                                    /* ------------------ GENERIC  MEMBERS ------------------ */
    OS_OBJ_TYPE          Type;                              /* Should be set to OS_OBJ_TYPE_FLAG                      */

    CPU_CHAR            *NamePtr;                           /* Pointer to Event Flag Name (NUL terminated ASCII)      */

    OS_PEND_LIST         PendList;                          /* List of tasks waiting on event flag group              */
                                                           /* ------------------ SPECIFIC MEMBERS ------------------ */
    OS_FLAGS             Flags;                             /* 8, 16 or 32 bit flags                                  */
    
    CPU_TS               TS;
};

OS_EXT CPU_TS      OSTmrTaskTimeMax;
OS_EXT OS_TICK     OSTmrTickCtr;               /* Current time for the timers                */
OS_EXT OS_CTR      OSTmrUpdateCnt;             /* Counter for updating timers                */
OS_EXT OS_CTR      OSTmrUpdateCtr;
OS_EXT OS_OBJ_QTY  OSTmrQty;
OS_EXT OS_RDY_LIST OSRdyList[OS_CFG_PRIO_MAX];

OS_EXT OS_PRIO OSPrioCur;
OS_EXT OS_PRIO OSPrioHighRdy;

OS_EXT OS_TCB *OSTCBCurPtr; 
OS_EXT OS_TCB *OSTCBHighRdyPtr; 


OS_EXT OS_STATE OSRunning;


OS_EXT OS_TCB OSIdleTaskTCB;

/* 空闲任务计数变量 */
OS_EXT OS_IDLE_CTR OSIdleTaskCtr;

/*记录系统启动到现在或者从上一次复位到现在经过了多少个SysTick周期*/
OS_EXT  OS_TICK    OSTickCtr;

OS_EXT  OS_NESTING_CTR  OSIntNestingCtr;        /* Interrupt nesting level */ 
OS_EXT  OS_NESTING_CTR  OSSchedLockNestingCtr;  /* Lock nesting leve */

OS_EXT  OS_OBJ_QTY      OSSemQty;

/* 空闲任务堆栈起始地址 */
extern CPU_STK * const OSCfg_IdleTaskStkBasePtr;

/* 空闲任务堆栈大小 */
extern CPU_STK_SIZE const OSCfg_IdleTaskStkSize;


#define OS_STATE_OS_STOPPED (OS_STATE)(0u)
#define OS_STATE_OS_RUNNING (OS_STATE)(1u)

#define  OS_TMR_STATE_UNUSED                    (OS_STATE)(0u)
#define  OS_TMR_STATE_STOPPED                   (OS_STATE)(1u)
#define  OS_TMR_STATE_RUNNING                   (OS_STATE)(2u)
#define  OS_TMR_STATE_COMPLETED                 (OS_STATE)(3u)

#define  OS_OPT_TMR_ONE_SHOT                      (OS_OPT)(1u)  
#define  OS_OPT_TMR_PERIODIC                      (OS_OPT)(2u)

#define  OS_OPT_DEL_NO_PEND                  (OS_OPT)(0x0000u)
#define  OS_OPT_DEL_ALWAYS                   (OS_OPT)(0x0001u)

#define  OS_OPT_PEND_FLAG_MASK               (OS_OPT)(0x000Fu)
#define  OS_OPT_PEND_FLAG_CLR_ALL            (OS_OPT)(0x0001u)  /* Wait for ALL    the bits specified to be CLR       */
#define  OS_OPT_PEND_FLAG_CLR_AND            (OS_OPT)(0x0001u)

#define  OS_OPT_PEND_FLAG_CLR_ANY            (OS_OPT)(0x0002u)  /* Wait for ANY of the bits specified to be CLR       */
#define  OS_OPT_PEND_FLAG_CLR_OR             (OS_OPT)(0x0002u)

#define  OS_OPT_PEND_FLAG_SET_ALL            (OS_OPT)(0x0004u)  /* Wait for ALL    the bits specified to be SET       */
#define  OS_OPT_PEND_FLAG_SET_AND            (OS_OPT)(0x0004u)

#define  OS_OPT_PEND_FLAG_SET_ANY            (OS_OPT)(0x0008u)  /* Wait for ANY of the bits specified to be SET       */
#define  OS_OPT_PEND_FLAG_SET_OR             (OS_OPT)(0x0008u)

#define  OS_OPT_PEND_FLAG_CONSUME            (OS_OPT)(0x0100u)  /* Consume the flags if condition(s) satisfied        */


#define  OS_OPT_PEND_BLOCKING                (OS_OPT)(0x0000u)
#define  OS_OPT_PEND_NON_BLOCKING            (OS_OPT)(0x8000u)

#define  OS_OPT_POST_ALL                     (OS_OPT)(0x0200u) 
#define  OS_OPT_POST_NO_SCHED                (OS_OPT)(0x8000u)

#define  OS_OPT_POST_FLAG_SET                (OS_OPT)(0x0000u)
#define  OS_OPT_POST_FLAG_CLR                (OS_OPT)(0x0001u)

#define OS_PRIO_TBL_SIZE ((OS_CFG_PRIO_MAX - 1u) / (32) + 1u)

extern CPU_DATA OSPrioTbl[OS_PRIO_TBL_SIZE];

struct  os_tmr_spoke {
    OS_TMR              *FirstPtr;                          /* Pointer to first timer in linked list                  */
    OS_OBJ_QTY           NbrEntries;
    OS_OBJ_QTY           NbrEntriesMax;
};


struct os_tick_spoke 
{
    OS_TCB *FirstPtr;
    OS_OBJ_QTY NbrEntries;
    OS_OBJ_QTY NbrEntriesMax;
};

extern OS_TICK_SPOKE OSCfg_TickWheel[];
extern OS_OBJ_QTY const OSCfg_TickWheelSize;

extern OS_TMR_SPOKE OSCfg_TmrWheel[];
extern OS_OBJ_QTY const OSCfg_TmrWheelSize;


typedef void (*OS_TASK_PTR)(void *p_arg);



///////////////////////////////////////////////////操作系统对外接口//////////////////////////////////////////////////////

void          OSStart                   (OS_ERR                *p_err);
void          OSInit                    (OS_ERR                *p_err);


void         OSTaskCreate               (OS_TCB       *p_tcb,
                                         OS_TASK_PTR  p_task,
                                         void         *p_arg,
                                         OS_PRIO      prio,
                                         CPU_STK      *p_stk_base,
                                         CPU_STK_SIZE stk_size,
                                         OS_TICK      time_quanta,
                                         OS_ERR       *p_err);
                                         
                                         
CPU_STK     *OSTaskStkInit               (OS_TASK_PTR p_task, 
                                         void *p_arg, 
                                         CPU_STK *p_stk_base, 
                                         CPU_STK_SIZE stk_size);
                                         
void        OSTimeTick                   (void);  
void        OSTimeDly                    (OS_TICK dly);
void        OSSched                      (void);   
void        OS_CPU_SysTickInit           (CPU_INT32U ms);      
void        OS_IdleTask                  (void *p_arg);     
void        OS_PrioInit                  (void );
void        OS_PrioInsert                (OS_PRIO prio);
void        OS_PrioRemove                (OS_PRIO prio);
OS_PRIO     OS_PrioGetHighest             (void);       
void        OS_RdyListInsertTail         (OS_TCB *p_tcb);
void        OS_RdyListInsertHead         (OS_TCB *p_tcb);   
void        OS_RdyListRemove             (OS_TCB *p_tcb);
void        OS_RdyListMoveHeadToTail     (OS_RDY_LIST *p_rdy_list);
void        OS_RdyListInsert             (OS_TCB *p_tcb);  
void        OS_TickListUpdate            (void);
void        OS_TickListRemove            (OS_TCB *p_tcb);
void        OS_TickListInsert            (OS_TCB *p_tcb,OS_TICK time);  
void        OS_TaskRdy                   (OS_TCB *p_tcb);
                                         
#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u
void        OS_SchedRoundRobin           (OS_RDY_LIST *p_rdy_list);
#endif

#if OS_CFG_TASK_SUSPEND_EN > 0u
void        OSTaskSuspend                (OS_TCB *p_tcb,OS_ERR *p_err);
void        OSTaskResume                 (OS_TCB *p_tcb,OS_ERR *p_err);
#endif


void        OS_TmrInit                   (OS_ERR  *p_err);

void        OSTmrCreate                  (OS_TMR               *p_tmr,          //定时器控制块指针
                                         CPU_CHAR             *p_name,         //命名定时器，有助于调试
                                         OS_TICK               dly,            //初始定时节拍数
                                         OS_TICK               period,         //周期定时重载节拍数
                                         OS_OPT                opt,            //选项
                                         OS_TMR_CALLBACK_PTR   p_callback,     //定时到期时的回调函数
                                         void                 *p_callback_arg, //传给回调函数的参数
                                         OS_ERR               *p_err);      
                                         
CPU_BOOLEAN  OSTmrDel                    (OS_TMR  *p_tmr, //定时器控制块指针
                                         OS_ERR  *p_err); //返回错误类型   
                                         
CPU_BOOLEAN  OSTmrStart                  (OS_TMR  *p_tmr,  //定时器控制块指针
                                         OS_ERR  *p_err);  //返回错误类型 

CPU_BOOLEAN  OSTmrStop                   (OS_TMR  *p_tmr,          //定时器控制块指针
                                         OS_OPT   opt,            //选项
                                         void    *p_callback_arg, //传给回调函数的新参数
                                         OS_ERR  *p_err);          //返回错误类型
                                         
void        OS_Post                      (OS_PEND_OBJ  *p_obj,     //内核对象类型指针
                                         OS_TCB       *p_tcb,     //任务控制块
                                         void         *p_void,    //消息
                                         OS_MSG_SIZE   msg_size,  //消息大小
                                         CPU_TS        ts);        //时间戳
                                         
void        OS_Pend                      (OS_PEND_OBJ   *p_obj,        //等待的内核对象
                                         OS_STATE       pending_on,   //等待哪种对象内核
                                         OS_TICK        timeout);      //等待期限 
                                          


void        OS_PendObjDel                (OS_PEND_OBJ  *p_obj,  //被删除对象的类型
                                         OS_TCB       *p_tcb,   //任务控制块指针
                                         CPU_TS        ts);     //信号量被删除时的时间戳
                                         
void        OS_PendListInit              (OS_PEND_LIST  *p_pend_list);

void        OSSemCreate                  (OS_SEM      *p_sem,  //多值信号量控制块指针
                                         CPU_CHAR    *p_name,  //多值信号量名称
                                         OS_SEM_CTR   cnt,     //资源数目或事件是否发生标志
                                         OS_ERR      *p_err);  //返回错误类型
                                         
OS_SEM_CTR  OSSemPend                    (OS_SEM   *p_sem,  //多值信号量指针
                                         OS_TICK   timeout, //等待超时时间
                                         OS_OPT    opt,     //选项
                                         CPU_TS   *p_ts,    //等到信号量时的时间戳
                                         OS_ERR   *p_err);  //返回错误类型    
                                         
OS_SEM_CTR  OSSemPost                    (OS_SEM  *p_sem,   //多值信号量控制块指针
                                         OS_OPT   opt,      //选项
                                         OS_ERR  *p_err);   //返回错误类型
                                         
void        OS_SemInit                   (OS_ERR  *p_err);

void        OS_PendListChangePrio        (OS_TCB  *p_tcb);

OS_PRIO     OS_MutexGrpPrioFindHighest   (OS_TCB  *p_tcb);

void        OS_TaskChangePrio            (OS_TCB  *p_tcb, OS_PRIO  prio_new);

void        OS_PendListRemove            (OS_TCB  *p_tcb);

void        OS_PendAbort                 (OS_TCB     *p_tcb,
                                         CPU_TS      ts,
                                         OS_STATUS   reason);

void        OSMutexCreate                (OS_MUTEX  *p_mutex,
                                         CPU_CHAR  *p_name,
                                         OS_ERR    *p_err);
                                         
void        OSMutexPend                  (OS_MUTEX  *p_mutex,
                                         OS_TICK    timeout,
                                         OS_OPT     opt,
                                         CPU_TS    *p_ts,
                                         OS_ERR    *p_err);                                        

void        OSMutexPost                  (OS_MUTEX  *p_mutex,
                                         OS_OPT     opt,
                                         OS_ERR    *p_err);
#endif

                                         
 







  
















