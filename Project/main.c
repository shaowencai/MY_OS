#include "os.h"
#include "ARMCM3.h"
#include "lib_std.h"
#include "heap.h"
#include "stdlib.h"
#include "stdio.h"
#define TASK1_STK_SIZE 128 
#define TASK2_STK_SIZE 128 
#define TASK3_STK_SIZE 128 

static CPU_STK Task1Stk[TASK1_STK_SIZE]; //ucos的栈大小都是以字为单位
static CPU_STK Task2Stk[TASK2_STK_SIZE];
static CPU_STK Task3Stk[TASK3_STK_SIZE];

static OS_TCB Task1TCB;
static OS_TCB Task2TCB;
static OS_TCB Task3TCB;

uint32_t flag1;
uint32_t flag2;
uint32_t flag3;
uint32_t flag5 = 0;
int *ccc;
int *bbb;
void Task1( void *p_arg );
void Task2( void *p_arg );
void Task3( void *p_arg );

uint32_t flag4 = 0;
void Buz_Init(void);
//--keep=__fsym* --keep=__vsym*

#if 0  //可以计算时间戳
uint32_t TimeStart; /* 定义三个全局变量 */ 
uint32_t TimeEnd;
uint32_t TimeUse;
#endif 

static OS_TMR   alarm_timer;
static OS_SEM   sem_ok;
static OS_MUTEX mutex_ok;

int main(void)
{ 
    OS_ERR err = OS_ERR_NONE;
	
	
	  long long i = 0;
		void *p[125];
		void *b =NULL;
	
		for(i=0; i<126; i++)
		{
			p[i] = malloc(1024);
		}

		for(i=0; i<120; i++)
		{
			free(p[i]);
			p[i] = 0;
		}
		
		b = malloc(1024 *10);
		
		
    CPU_Init();
    CPU_IntDis();
    OS_CPU_SysTickInit(100);
    My_Mem_Init();

    /* 初始化相关的全局变量 */ 
    OSInit(&err);
  
    OSSemCreate (&sem_ok,"sss", 0,&err);
    
    OSMutexCreate (&mutex_ok,"aaa",&err); 
    /* 创建任务 */ 
    OSTaskCreate ((OS_TCB*) &Task1TCB, 
                  (OS_TASK_PTR ) Task1, 
                  (void *) 0, 
                  (OS_PRIO ) 5,
                  (CPU_STK*) &Task1Stk[0], 
                  (CPU_STK_SIZE) TASK1_STK_SIZE, 
                  (OS_TICK )1,
                  (OS_ERR *) &err); 
                 
    OSTaskCreate ((OS_TCB*) &Task2TCB, 
                  (OS_TASK_PTR ) Task2, 
                  (void *) 0, 
                  (OS_PRIO ) 2,
                  (CPU_STK*) &Task2Stk[0], 
                  (CPU_STK_SIZE) TASK2_STK_SIZE, 
                  (OS_TICK ) 1,
                  (OS_ERR *) &err);              

   OSTaskCreate ((OS_TCB*) &Task3TCB, 
                  (OS_TASK_PTR ) Task3, 
                  (void *) 0, 
                  (OS_PRIO ) 1,
                  (CPU_STK*) &Task3Stk[0], 
                  (CPU_STK_SIZE) TASK3_STK_SIZE, 
                  (OS_TICK ) 1,
                  (OS_ERR *) &err); 
                  
    Buz_Init();
    
    OSStart(&err);
}

void Buz_AlarmTimeout(void *p_tmr, void *p_arg)
{
    flag4++;
    if(flag4 == 2)flag4 = 0;
}



void Buz_Init(void)
{
    OS_ERR err;

    OSTmrCreate (&alarm_timer,"alarm_tmr",0,1,\
                 OS_OPT_TMR_PERIODIC,Buz_AlarmTimeout,0,&err);
    
    OSTmrStart(&alarm_timer,&err);
}


void delay()
{
    unsigned char  i=0;
    for(i=0;i<0xff;i++);
    for(i=0;i<0xff;i++);
}

void Task1( void *p_arg ) 
{ 
    OS_ERR err;
    for ( ;; ) 
    { 
        flag1 = 1; 
			  ccc = (int *)malloc(sizeof(int)*200);
				if(ccc == bbb && bbb != 0)
				{
					flag5 = 1;
					while(1);
				}
				delay();
				delay();
			  free(ccc);
				ccc = 0;
        //TimeStart = OS_TS_GET();
        //OSTaskSuspend(&Task1TCB,&err);
        //OSTimeDly(1);
        //TimeEnd = OS_TS_GET();
        //TimeUse = TimeEnd - TimeStart;
        flag1 = 0;
       // OSTaskSuspend(&Task1TCB,&err);//OSTimeDly(2);
    }
}

void Task2( void *p_arg ) 
{ 
    OS_ERR err;

    for ( ;; ) 
    { 
        //OSMutexPend(&mutex_ok,0,OS_OPT_PEND_BLOCKING,0,&err);
        flag2 = 1; 
				bbb = (int *)malloc(sizeof(int)*200);
				if(ccc == bbb && bbb != 0)
				{
					flag5 = 1;
					while(1);
				}
			  OSTimeDly(1);
			  free(bbb);
			  bbb = 0;
        flag2 = 0; 
        OSTimeDly(1);
        //OSTaskResume(&Task1TCB,&err);
        //OSMutexPost (&mutex_ok,OS_OPT_POST_NO_SCHED,&err);
        //OSSemPost(&sem_ok,OS_OPT_POST_ALL,&err);
    }
}




void Task3( void *p_arg ) 
{ 
    OS_ERR err;
    for ( ;; ) 
    { 
        
        //OSMutexPend(&mutex_ok,0,OS_OPT_PEND_BLOCKING,0,&err);
        flag3 = 1;
        OSTimeDly(1);
        //OSSemPend(&sem_ok,1,OS_OPT_PEND_BLOCKING,0,&err);
        flag3 = 0;
        //OSMutexPost (&mutex_ok,OS_OPT_POST_NO_SCHED,&err);
        //OSSemPend(&sem_ok,1,OS_OPT_PEND_BLOCKING,0,&err);
    }

}












//typedef struct
//{
//    uint16_t reg;
//    uint16_t ram;
//}lcd_typedef;

//#define LCD_BASE ((uint32_t)(0x6c000000|0x0000007e))

//#define LCD  ((lcd_typedef *)LCD_BASE)

//void lcd_wr_reg(uint16_t val)
//{
//    val = val;
//    LCD->reg = val;
//}
//void lcd_wr_data(uint16_t val)
//{
//    val = val;
//    LCD->ram = val;
//}

//int *a = 0;
//int *b = 0;
//int *c = 0;
//    a= (int *)My_malloc(10);
//b= (int *)My_malloc(10);
//c= (int *)My_malloc(10);
//My_free(a);

//a = 0;
//a= (int *)My_malloc(10);

//My_free(a);

//char a[4] = {0};
//char *p=0;
//int *p1;
//    
//    p = &a[2];
//    
//    p1 = (int *)p;
//    *p1 = 0x12345678;
//    p++;
//    p =(char *)p1;
//    *p = 13;

//结论  高字节对应高地址
//int TTTTT;
//char Q1;
//char Q2;
//char Q3;
//char Q4;

//    TTTTT = 0x12345678;
//    p = (char *)&TTTTT;
//    
//    Q1 = *p;p++;
//    Q2 = *p;p++;
//    Q3 = *p;p++;
//    Q4 = *p;

//压栈的时候 sp是--；
// psuh == sp--

//#include <stdio.h>

//int stack_dir;  
//void find_stack_direction (void) 
//{  
//    static char   *addr = NULL;                                     
//    char     dummy;        

//    if (addr == NULL)  
//    {                           /* initial entry */  
//        addr = &dummy;  

//        find_stack_direction ();  /* recurse once */  
//    }  
//    else                          /* second entry */  
//        if (&dummy > addr)  
//            stack_dir = 1;            /* stack grew upward */  
//        else  
//            stack_dir = -1;           /* stack grew downward */  
//}
//    find_stack_direction();  
//    if(stack_dir==1)  stack_dir =1;

//    else  stack_dir = - 1;




//union Batcher_Arbitration shaowencai;
//int a;
//    shaowencai.Arbitration_t.prepare_solid_a =1;
//     shaowencai.Arbitration_t.prepare_solid_b =1;
//    a = sizeof(shaowencai);
//    
//    a= shaowencai.var;
//    shao.c =1;
//    
//    wen = (int*)&shao.b;
//    wen = (int*)&shao.c;
//    wen = (int *)&yyy;
//struct a
//{
//    int c;
//    const int b;
//};

//struct a shao;

//const int yyy;
//int * wen;

//// int    prepare_solid_a:1; 占最低位
////int    prepare_solid_b:1;次低位

//union Batcher_Arbitration
//{
//  struct Arbitration
//  {
//     char    prepare_solid_a:1;
//     char    prepare_solid_b:1;
//     char    prepare_solid_c:1;
//     char    prepare_liquid_a:1;
//     char    prepare_liquid_b:1;
//     char    prepare_solid_a1:1;
//     char    prepare_solid_b1:1;
//     char    prepare_solid_c1:1;
//     char    prepare_liquid_a1:1;
//     char    prepare_liquid_b1:1;      
//     char    prepare_solid_a2:1;
//     char    prepare_solid_b2:1;
//     char    prepare_solid_c2:1;
//     int    prepare_liquid_a2:1;
//     int    prepare_liquid_b2:1;  
//     int    prepare_solid_a3:1;
//     int    prepare_solid_b3:1;
//     int    prepare_solid_c3:1;
//     int    prepare_liquid_a3:1;
//     int    prepare_liquid_b3:1;     
//     int    prepare_solid_a4:1;
//     int    prepare_solid_b4:1;
//     int    prepare_solid_c4:1;
//     int    prepare_liquid_a4:1;
//     int    prepare_liquid_b4:1; 
//     int    prepare_solid_a5:1;
//     int    prepare_solid_b5:1;
//     int    prepare_solid_c5:1;
//     int    prepare_liquid_a5:1;
//     int    prepare_liquid_b5:1; 
//     int    prepare_solid_a6:1;
//     int    prepare_solid_b6:1;
//     
//  }Arbitration_t;

//   int var;
//};


//extern const int FSymTab$$Base;
//extern const int FSymTab$$Limit;

//extern const int VSymTab$$Base;
//extern const int VSymTab$$Limit;
//    

//#define SECTION(x)                  __attribute__((section(x)))

//typedef long (*syscall_func)();

//struct finsh_syscall
//{
//	const char*		name;		/* the name of system call */
//	const char*		desc;		/* description of system call */
//	syscall_func        func;		/* the function address of system call */
//};

//struct finsh_syscall* finsh_syscall_lookup(const char* name);

//struct finsh_sysvar
//{
//	const char*		 name;		/* the name of variable */
//	const char* 	     desc;		/* description of system variable */
//	unsigned char		 type;		/* the type of variable */
//	void*		          var;		/* the address of variable */
//};


//#define FINSH_FUNCTION_EXPORT_CMD(name, cmd, desc)      \
//    const char __fsym_##cmd##_name[] SECTION(".rodata.name") = #cmd;   \
//    const char __fsym_##cmd##_desc[] SECTION(".rodata.name") = #desc;  \
//    const struct finsh_syscall __fsym_##cmd SECTION("FSymTab")= \
//    {                           \
//        __fsym_##cmd##_name,    \
//        __fsym_##cmd##_desc,    \
//        (syscall_func)&name     \
//    };

//#define FINSH_VAR_EXPORT(name, type, desc)              \
//    const char __vsym_##name##_name[] SECTION(".rodata.name") = #name; \
//    const char __vsym_##name##_desc[] SECTION(".rodata.name") = #desc; \
//    const struct finsh_sysvar __vsym_##name SECTION("VSymTab")= \
//    {                           \
//        __vsym_##name##_name,   \
//        __vsym_##name##_desc,   \
//        type,                   \
//        (void*)&name            \
//    };
//                
//                
//#define FINSH_FUNCTION_EXPORT(name, desc)  FINSH_FUNCTION_EXPORT_CMD(name, name, desc)
//    
//#define FINSH_FUNCTION_EXPORT_ALIAS(name, alias, desc)  FINSH_FUNCTION_EXPORT_CMD(name, alias, desc)
//    
//    
//void led()
//{
//    int a;
//    a++;
//}
//FINSH_FUNCTION_EXPORT(led,led);

//int c;

//FINSH_VAR_EXPORT(c,1,var);



