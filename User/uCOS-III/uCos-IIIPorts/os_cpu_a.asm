

NVIC_INT_CTRL   EQU 0xE000ED04 ; �жϿ��Ƽ�״̬�Ĵ��� SCB_ICSR
NVIC_SYSPRI14   EQU 0xE000ED22 ; ϵͳ���ȼ��Ĵ��� SCB_SHPR3��
NVIC_PENDSV_PRI EQU 0xFF       ; PendSV ���ȼ���ֵ(���)��
NVIC_PENDSVSET  EQU 0x10000000 ; ����PendSV�쳣��ֵ Bit28��PENDSVSET    
 
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
   LDR  R0,= NVIC_SYSPRI14   ; ϵͳ���ȼ��Ĵ���
   LDR  R1,= NVIC_PENDSV_PRI ; ������ȼ�
   STRB R1, [R0]; ���� PendSV �쳣���ȼ�Ϊ���
   
   MOVS R0, #0 ;����psp��ֵΪ0����ʼ��һ���������л�
   MSR PSP, R0
   
   LDR R0, =NVIC_INT_CTRL
   LDR R1, =NVIC_PENDSVSET
   STR R1, [R0] ; ����PendSV�쳣
   
   CPSIE I ;ʹ�����жϣ�NMI��HardFault����
OSStartHang
   B OSStartHang ; ����Ӧ��Զ�������е�����
   



        
PendSV_Handler
    CPSID I
    MRS R0, PSP
    CBZ R0, OS_CPU_PendSVHandler_nosave
    
    ;R0��Ϊ0��˵�����ǵ�һ���л���
    ;������л���������һ��Ҫ���е�����Ķ�ջ���ݼ��ص�CPU�Ĵ�����
    ;�ڽ���PendSV�쳣��ʱ�򣬵�ǰCPU��xPSR��PC��������ڵ�ַ����
    ;R14��R12��R3��R2��R1��R0���Զ��洢����ǰ�����ջ
    STMDB R0!, {R4-R11};�ֶ��洢CPU�Ĵ���R4-R11��ֵ����ǰ����Ķ�ջ
    LDR R1, = OSTCBCurPtr ;����ָ�������ַ
    LDR R1, [R1]          ;��ָ������д�ŵĵ�ַ��R1 �������R1������ݾ��ǵ�ǰ������ƿ�ĵ�ַ
    STR R0, [R1]          ;���ƿ���׵�ַҲ���������ջָ������ĵ�ַ����R0�����ջ��������ջָ�����
    
OS_CPU_PendSVHandler_nosave

    ; OSPrioCur = OSPrioHighRdy 
    LDR R0, =OSPrioCur 
    LDR R1, =OSPrioHighRdy 
    LDRB R2, [R1] 
    STRB R2, [R0]

    LDR R0, = OSTCBCurPtr      ;����ָ�������ַ
    LDR R1, = OSTCBHighRdyPtr

    LDR R2, [R1]    ;��ָ������д�ŵĵ�ַ��R2
    STR R2, [R0]

    LDR R0, [R2]    ;�����ƿ�ĵ�һ�����ݼ��ص�R0 Ҳ����ջ��ָ��
    LDMIA R0!, {R4-R11} ;��ջ��ָ��ָ����������������R4-R11�Ĵ�����

    MSR PSP, R0  ;��ͨ�üĴ��������ݴ���������Ĵ���
    
    ; ȷ���쳣����ʹ�õĶ�ջָ����PSP����LR�Ĵ�����λ2ҪΪ1
    ORR LR, LR, #0x04

    CPSIE I

    BX LR      ; �쳣����
    
    ALIGN
    END  

   