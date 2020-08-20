#include "os.h"

CPU_DATA OSPrioTbl[OS_PRIO_TBL_SIZE];

void OS_PrioInit( void )
{
	CPU_DATA i;
	for ( i=0u; i<OS_PRIO_TBL_SIZE; i++ ) 
	{
		OSPrioTbl[i] = (CPU_DATA)0;
	}
}



void OS_PrioInsert (OS_PRIO prio)
{
	CPU_DATA bit;
	CPU_DATA bit_nbr;
	OS_PRIO ix;

	ix = prio / 32;
	bit_nbr = (CPU_DATA)prio & (32 - 1u);

	bit = 1u;
	bit <<= (32 - 1u) - bit_nbr;

	OSPrioTbl[ix] |= bit;
}


void OS_PrioRemove (OS_PRIO prio)
{
	CPU_DATA bit;
	CPU_DATA bit_nbr;
	OS_PRIO ix;

	ix = prio / 32;
	bit_nbr = (CPU_DATA)prio & (32 - 1u);

	bit = 1u;
	bit <<= (32 - 1u) - bit_nbr;

	OSPrioTbl[ix] &= ~bit;
}



OS_PRIO OS_PrioGetHighest (void)
{
	CPU_DATA *p_tbl;
	OS_PRIO prio;

	prio = (OS_PRIO)0;
	p_tbl = &OSPrioTbl[0];
	while (*p_tbl == (CPU_DATA)0) 
	{
		prio += 32; 
		p_tbl++;
	}

	prio += (OS_PRIO)CPU_CntLeadZeros(*p_tbl);
	return (prio);
}



