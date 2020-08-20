#ifndef CPU_H 
#define CPU_H  

typedef            char                  CPU_CHAR;  
typedef  unsigned short                  CPU_INT16U; 
typedef  unsigned int                    CPU_INT32U;
typedef  unsigned char                   CPU_INT08U; 
typedef  unsigned long long              CPU_INT64U;
typedef  unsigned char                   CPU_BOOLEAN;

typedef CPU_INT32U                       CPU_ADDR; 
typedef CPU_INT32U                       CPU_STK;  

typedef CPU_ADDR                         CPU_STK_SIZE; 
typedef volatile                        CPU_INT32U CPU_REG32;
typedef  CPU_INT32U                      CPU_DATA;
typedef  CPU_INT32U                      CPU_SR;
 

#endif
