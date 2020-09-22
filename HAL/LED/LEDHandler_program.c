/*
 *
 *  Created on: Sep 22, 2020
 *      Author: Youssef Kamal
 */
#include "STD_Types.h"
#include "RCC_interface.h"
#include "LEDHandler_interface.h"

extern LEDHandlerConfigType ledsConfig[NUMBER_OF_LEDS];

void LED_init(void)
{
	u8 i;
	for(i=0;i<NUMBER_OF_LEDS;i++)
	{
		switch(ledsConfig[i].ledPin.port)
		{
		case LEDH_A_PORT:
			RCC_u8_ControlPrephiral(RCC_u32_CONTROL_PERIP_IOPAEN,RCC_u8_ENABLE);
			break;
		case LEDH_B_PORT:
			RCC_u8_ControlPrephiral(RCC_u32_CONTROL_PERIP_IOPBEN,RCC_u8_ENABLE);
			break;
		case LEDH_C_PORT:
			RCC_u8_ControlPrephiral(RCC_u32_CONTROL_PERIP_IOPCEN,RCC_u8_ENABLE);
			break;
		case LEDH_D_PORT:
			RCC_u8_ControlPrephiral(RCC_u32_CONTROL_PERIP_IOPDEN,RCC_u8_ENABLE);
			break;
		case LEDH_E_PORT:
			RCC_u8_ControlPrephiral(RCC_u32_CONTROL_PERIP_IOPEEN,RCC_u8_ENABLE);
			break;
		case LEDH_F_PORT:
			RCC_u8_ControlPrephiral(RCC_u32_CONTROL_PERIP_IOPFEN,RCC_u8_ENABLE);
			break;
		case LEDH_G_PORT:
			RCC_u8_ControlPrephiral(RCC_u32_CONTROL_PERIP_IOPGEN,RCC_u8_ENABLE);
		}
		GPIO_configPins(&ledsConfig[i].ledPin);
		GPIO_setPinValue(ledsConfig[i].ledPin.port,ledsConfig[i].ledPin.pin,ledsConfig[i].initState^ledsConfig[i].activeState);
	}
}

void LED_setState(u8 ledID,u8 state)
{
	/*Check if input is valid*/
	if(ledID<NUMBER_OF_LEDS)
	{
		GPIO_setPinValue(ledsConfig[ledID].ledPin.port,ledsConfig[ledID].ledPin.pin,state^ledsConfig[ledID].activeState);
	}
}
