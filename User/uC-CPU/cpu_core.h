#ifndef CPU_COER_H
#define CPU_COER_H

#include "cpu.h"
#include "cpu_cfg.h"


#ifdef CPU_CORE_MODULE /* CPU_CORE_MODULE 只在cpu_core.c文件的开头定义 */
#define CPU_CORE_EXT
#else
#define CPU_CORE_EXT extern
#endif



//控制时间戳可以是32位或是64位的
#if ((CPU_CFG_TS_32_EN == 1) || \
     (CPU_CFG_TS_64_EN == 1))
#define CPU_CFG_TS_EN   1
#else
#define CPU_CFG_TS_EN   0
#endif
      

#if ((CPU_CFG_TS_EN == 1) || \
     (defined(CPU_CFG_INT_DIS_MEAS_EN)))
#define CPU_CFG_TS_TMR_EN 1
#else
#define CPU_CFG_TS_TMR_EN 0
#endif


typedef CPU_INT32U 	CPU_TS32;
typedef CPU_INT32U 	CPU_TS_TMR_FREQ;
typedef CPU_TS32 	CPU_TS;
typedef CPU_INT32U 	CPU_TS_TMR;


#if (CPU_CFG_TS_TMR_EN == 1) 
CPU_CORE_EXT CPU_TS_TMR_FREQ  CPU_TS_TmrFreq_Hz; //定义全局变量
#endif


#if (CPU_CFG_TS_TMR_EN == 1)
CPU_TS_TMR  CPU_TS_TmrRd (void); 
#endif

void CPU_Init (void);


#endif



