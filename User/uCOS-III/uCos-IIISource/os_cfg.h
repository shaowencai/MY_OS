#ifndef OS_CFG_H 
#define OS_CFG_H 

/* 支持最大的优先级 */ 
#define OS_CFG_PRIO_MAX                 32u 
#define OS_CFG_TS_EN                    1u
#define OS_CFG_SCHED_ROUND_ROBIN_EN     1u
#define OS_CFG_TASK_SUSPEND_EN          1u

#define OS_CFG_ARG_CHK_EN               1u   /* Enable (1) or Disable (0) argument checking                           */
#define OS_CFG_CALLED_FROM_ISR_CHK_EN   1u   /* Enable (1) or Disable (0) check for called from ISR                   */

#endif /* OS_CFG_H */

