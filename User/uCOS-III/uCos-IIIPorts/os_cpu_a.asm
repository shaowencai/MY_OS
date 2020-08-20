

NVIC_INT_CTRL   EQU 0xE000ED04 ; 中断控制及状态寄存器 SCB_ICSR
NVIC_SYSPRI14   EQU 0xE000ED22 ; 系统优先级寄存器 SCB_SHPR3：
NVIC_PENDSV_PRI EQU 0xFF       ; PendSV 优先级的值(最低)。
NVIC_PENDSVSET  EQU 0x10000000 ; 触发PendSV异常的值 Bit28：PENDSVSET    
 
    EXPORT OSStartHighRdy
    IMPORT OSTCBCurPtr
    IMPORT OSTCBHighRdyPtr
    IMPORT OSPrioCur
    IMPORT OSPrioHighRdy
    EXPORT PendSV_Handler  
    EXPORT  CPU_IntDis
    EXPORT  CPU_IntEn
    EXPORT CPU_CntLeadZeros
        
    EXPORT  CPU_SR_Save
    EXPORT  CPU_SR_Restore

    PRESERVE8
    THUMB

    AREA CODE, CODE, READONLY

CPU_SR_Save
        MRS     R0, PRIMASK                     ; Set prio int mask to mask all (except faults)
        CPSID   I
        BX      LR


CPU_SR_Restore                                  ; See Note #2.
        MSR     PRIMASK, R0
        BX      LR
        
        
CPU_CntLeadZeros 
    CLZ R0, R0
    BX LR

CPU_IntDis
        CPSID   I
        BX      LR


CPU_IntEn
        CPSIE   I
        BX      LR
        
        
OSStartHighRdy
   LDR  R0,= NVIC_SYSPRI14   ; 系统优先级寄存器
   LDR  R1,= NVIC_PENDSV_PRI ; 最低优先级
   STRB R1, [R0]; 设置 PendSV 异常优先级为最低
   
   MOVS R0, #0 ;设置psp的值为0，开始第一次上下文切换
   MSR PSP, R0
   
   LDR R0, =NVIC_INT_CTRL
   LDR R1, =NVIC_PENDSVSET
   STR R1, [R0] ; 触发PendSV异常
   
   CPSIE I ;使能总中断，NMI和HardFault除外
OSStartHang
   B OSStartHang ; 程序应永远不会运行到这里
   



        
PendSV_Handler
    CPSID I
    MRS R0, PSP
    CBZ R0, OS_CPU_PendSVHandler_nosave
    
    ;R0不为0，说明不是第一次切换。
    ;任务的切换，即把下一个要运行的任务的堆栈内容加载到CPU寄存器中
    ;在进入PendSV异常的时候，当前CPU的xPSR，PC（任务入口地址），
    ;R14，R12，R3，R2，R1，R0会自动存储到当前任务堆栈
    STMDB R0!, {R4-R11};手动存储CPU寄存器R4-R11的值到当前任务的堆栈
    LDR R1, = OSTCBCurPtr ;加载指针变量地址
    LDR R1, [R1]          ;将指针变量中存放的地址给R1 这里就是R1存放数据就是当前任务控制块的地址
    STR R0, [R1]          ;控制块的首地址也就是任务堆栈指针变量的地址，将R0代表的栈顶赋给堆栈指针变量
    
OS_CPU_PendSVHandler_nosave

    ; OSPrioCur = OSPrioHighRdy 
    LDR R0, =OSPrioCur 
    LDR R1, =OSPrioHighRdy 
    LDRB R2, [R1] 
    STRB R2, [R0]

    LDR R0, = OSTCBCurPtr      ;加载指针变量地址
    LDR R1, = OSTCBHighRdyPtr

    LDR R2, [R1]    ;将指针变量中存放的地址给R2
    STR R2, [R0]

    LDR R0, [R2]    ;将控制块的第一个数据加载到R0 也就是栈顶指针
    LDMIA R0!, {R4-R11} ;将栈顶指针指向的数据依次输出到R4-R11寄存器中

    MSR PSP, R0  ;将通用寄存器的内容传送至特殊寄存器
    
    ; 确保异常返回使用的堆栈指针是PSP，即LR寄存器的位2要为1
    ORR LR, LR, #0x04

    CPSIE I

    BX LR      ; 异常返回
    
    ALIGN
    END  

   