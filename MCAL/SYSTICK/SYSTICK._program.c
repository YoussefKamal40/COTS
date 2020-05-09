#include "M3_interface.h"

#include "STD_Types_H.h"
#include "SYSTICK_interface.h"

void SysTick_Handler(void);

static SYSTICK_CBF_type interruptHandler;

void SYSTICK_init(SYSTICK_CBF_type callback,u32 preload,u8 clockSource)
{
	/*Disable timer*/
	SysTick->CTRL=0;
	/*Clear Timer*/
	SysTick->VAL=0;
	if(clockSource!=SYSTICK_CLK_SOURCE_EXT_REF)
	{
		SysTick->CTRL=SysTick_CTRL_CLKSOURCE_Msk;
	}
	/*Enable systick interrupt*/
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;
	/*Set preload value*/
	SysTick->LOAD=preload;
	/*SetCallback*/
	interruptHandler=callback;
}

void SYSTICK_deInit(void)
{
	SysTick->CTRL=0;
}

void SYSTICK_start(void)
{
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void)
{
	if(interruptHandler)
	interruptHandler();
}

u32 SYSTICK_current(void)
{
	return SysTick->VAL;
}
