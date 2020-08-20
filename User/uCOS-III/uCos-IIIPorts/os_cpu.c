#define   OS_CPU_GLOBALS

#include "os.h"


CPU_STK *OSTaskStkInit (OS_TASK_PTR p_task, 
                        void *p_arg, 
                        CPU_STK *p_stk_base, 
                        CPU_STK_SIZE stk_size)
{
    CPU_STK    *p_stk;
    p_stk = &p_stk_base[stk_size];                              /* Load stack pointer                                     */
                                                                /* Align the stack to 8-bytes.                            */
    p_stk = (CPU_STK *)((CPU_STK)(p_stk) & 0xFFFFFFF8);
                                                                /* Registers stacked as if auto-saved on exception        */
    *--p_stk = (CPU_STK)0x01000000u;                            /* xPSR                                                   */
    *--p_stk = (CPU_STK)p_task;                                 /* Entry Point                                            */
    *--p_stk = (CPU_STK)0x14141414u;                            /* R14 (LR)                                               */
    *--p_stk = (CPU_STK)0x12121212u;                            /* R12                                                    */
    *--p_stk = (CPU_STK)0x03030303u;                            /* R3                                                     */
    *--p_stk = (CPU_STK)0x02020202u;                            /* R2                                                     */
    *--p_stk = (CPU_STK)0x01010101u;                            /* R1                                                     */
    *--p_stk = (CPU_STK)p_arg;                                  /* R0 : argument                                          */
    
    /* 异常发生时需手动保存的寄存器 */
    *--p_stk = (CPU_STK)0x11111111u;                            /* R11                                                    */
    *--p_stk = (CPU_STK)0x10101010u;                            /* R10                                                    */
    *--p_stk = (CPU_STK)0x09090909u;                            /* R9                                                     */
    *--p_stk = (CPU_STK)0x08080808u;                            /* R8                                                     */
    *--p_stk = (CPU_STK)0x07070707u;                            /* R7                                                     */
    *--p_stk = (CPU_STK)0x06060606u;                            /* R6                                                     */
    *--p_stk = (CPU_STK)0x05050505u;                            /* R5                                                     */
    *--p_stk = (CPU_STK)0x04040404u;                            /* R4                                                     */
    
    return p_stk;
}


void OS_CPU_SysTickInit(CPU_INT32U ms)
{
    SysTick->LOAD = ms * SystemCoreClock / 1000 - 1;
    NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}



void SysTick_Handler(void)
{
    OSTimeTick();
}
