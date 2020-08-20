#ifndef OS_CPU_H
#define OS_CPU_H



#define  CPU_WORD_SIZE_08                          1u   /*  8-bit word size (in octets).                                */
#define  CPU_WORD_SIZE_16                          2u   /* 16-bit word size (in octets).                                */
#define  CPU_WORD_SIZE_32                          4u   /* 32-bit word size (in octets).                                */
#define  CPU_WORD_SIZE_64                          8u   /* 64-bit word size (in octets).                                */

    
#ifndef NVIC_INT_CTRL
#define NVIC_INT_CTRL *((CPU_REG32 *)0xE000ED04)
#endif

#ifndef NVIC_PENDSVSET
#define NVIC_PENDSVSET 0x10000000
#endif


#define OS_TASK_SW() NVIC_INT_CTRL = NVIC_PENDSVSET
#define OSIntCtxSw() NVIC_INT_CTRL = NVIC_PENDSVSET

void    OSStartHighRdy		(void);/* 在os_cpu_a.s中实现 */
void    PendSV_Handler		(void);/* 在os_cpu_a.s中实现 */
void    CPU_IntDis    		(void);/* 在os_cpu_a.s中实现 */
void    CPU_IntEn     		(void);/* 在os_cpu_a.s中实现 */
OS_PRIO  CPU_CntLeadZeros   (CPU_DATA p);

CPU_SR      CPU_SR_Save      (void);
void        CPU_SR_Restore   (CPU_SR      cpu_sr);

#define     CPU_SR_ALLOC()        CPU_SR  cpu_sr = (CPU_SR)0
#define     CPU_INT_DIS()         do { cpu_sr = CPU_SR_Save(); } while (0) /* Save    CPU status word & disable interrupts.*/
#define     CPU_INT_EN()          do { CPU_SR_Restore(cpu_sr); } while (0) /* Restore CPU status word.  */

#define     CPU_CRITICAL_ENTER()  CPU_INT_DIS() 

#define     CPU_CRITICAL_EXIT()   CPU_INT_EN() 


#if      OS_CFG_TS_EN == 1u
#define  OS_TS_GET()               (CPU_TS)CPU_TS_TmrRd() 
#else
#define  OS_TS_GET()               (CPU_TS)0u
#endif

#endif

