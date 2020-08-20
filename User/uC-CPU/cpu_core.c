
#define CPU_CORE_MODULE
#include "cpu_core.h"

/* 
******************************************************************* 
*                             寄存器定义  
******************************************************************* 
*/ 
#define BSP_REG_DEM_CR                          (*(CPU_REG32 *)0xE000EDFC) 
#define BSP_REG_DWT_CR                          (*(CPU_REG32 *)0xE0001000) 
#define BSP_REG_DWT_CYCCNT                      (*(CPU_REG32 *)0xE0001004) 
#define BSP_REG_DBGMCU_CR                       (*(CPU_REG32 *)0xE0042004)

#define BSP_DBGMCU_CR_TRACE_IOEN_MASK           0x10 
#define BSP_DBGMCU_CR_TRACE_MODE_ASYNC          0x00
#define BSP_DBGMCU_CR_TRACE_MODE_SYNC_01        0x40 
#define BSP_DBGMCU_CR_TRACE_MODE_SYNC_02        0x80 
#define BSP_DBGMCU_CR_TRACE_MODE_SYNC_04        0xC0 
#define BSP_DBGMCU_CR_TRACE_MODE_MASK           0xC0 
#define BSP_BIT_DEM_CR_TRCENA                   (1<<24)
#define BSP_BIT_DWT_CR_CYCCNTENA                (1<<0)


CPU_INT32U  BSP_CPU_ClkFreq (void)//获得系统时钟
{
    #if 0
    CPU_INT32U  hclk_freq;
    hclk_freq = HAL_RCC_GetHCLKFreq();
    return (hclk_freq);
    #endif
    
    #if 0
    RCC_ClocksTypeDef rcc_clocks; 
    RCC_GetClocksFreq(&rcc_clocks);
    return ((CPU_INT32U)rcc_clocks.HCLK_Frequency);
    #endif
    
    /* 目前软件仿真我们使用25M的系统时钟 */
    return 25000000;
    
}

#if (CPU_CFG_TS_TMR_EN == 1)
void  CPU_TS_TmrFreqSet (CPU_TS_TMR_FREQ  freq_hz)
{
    CPU_TS_TmrFreq_Hz = freq_hz;
}
#endif


#if (CPU_CFG_TS_TMR_EN == 1)
CPU_TS_TMR  CPU_TS_TmrRd (void)
{
    CPU_TS_TMR  ts_tmr_cnts;

    ts_tmr_cnts = (CPU_TS_TMR)BSP_REG_DWT_CYCCNT;

    return (ts_tmr_cnts);
}
#endif






#if (CPU_CFG_TS_TMR_EN == 1)
void CPU_TS_TmrInit (void)
{
    CPU_INT32U fclk_freq;
    fclk_freq = BSP_CPU_ClkFreq();
    BSP_REG_DEM_CR |= (CPU_INT32U)BSP_BIT_DEM_CR_TRCENA;
    BSP_REG_DWT_CYCCNT = (CPU_INT32U)0u;
    BSP_REG_DWT_CR |= (CPU_INT32U)BSP_BIT_DWT_CR_CYCCNTENA;
    CPU_TS_TmrFreqSet((CPU_TS_TMR_FREQ)fclk_freq);
}
#endif






#if ((CPU_CFG_TS_EN == 1) || \
     (CPU_CFG_TS_TMR_EN == 1))
static void CPU_TS_Init (void)
{
#if (CPU_CFG_TS_TMR_EN == 1)
    CPU_TS_TmrFreq_Hz = 0u;
    CPU_TS_TmrInit();
#endif
}
#endif



void CPU_Init (void)
{
#if ((CPU_CFG_TS_EN == 1) || \
     (CPU_CFG_TS_TMR_EN == 1))
    CPU_TS_Init();
#endif
}
    

