#include "os.h"


CPU_STK             OSCfg_IdleTaskStk[OS_CFG_IDLE_TASK_STK_SIZE]; 
CPU_STK * const    OSCfg_IdleTaskStkBasePtr = (CPU_STK *)&OSCfg_IdleTaskStk[0];
CPU_STK_SIZE const OSCfg_IdleTaskStkSize = (CPU_STK_SIZE)OS_CFG_IDLE_TASK_STK_SIZE;

OS_TICK_SPOKE       OSCfg_TickWheel[OS_CFG_TICK_WHEEL_SIZE];
OS_OBJ_QTY const   OSCfg_TickWheelSize = (OS_OBJ_QTY )OS_CFG_TICK_WHEEL_SIZE;


OS_TMR_SPOKE        OSCfg_TmrWheel[OS_CFG_TMR_WHEEL_SIZE];
OS_OBJ_QTY const    OSCfg_TmrWheelSize = (OS_OBJ_QTY )OS_CFG_TMR_WHEEL_SIZE;


//ø’œ–»ŒŒÒ
void OS_IdleTask (void *p_arg)
{
    p_arg = p_arg;
    for (;;) 
    {
        OSIdleTaskCtr++;
    }
}






