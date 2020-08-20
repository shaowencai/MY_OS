#ifndef __LIB_STD__
#define __LIB_STD__

#include "os.h"

void My_Mem_Init(void);
void *My_malloc( CPU_INT32U length  );
void My_free( void *pv );
void *My_realloc(void *rmem, CPU_INT32U newsize);

#endif

