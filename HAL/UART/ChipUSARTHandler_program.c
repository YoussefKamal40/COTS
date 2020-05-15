#include "STD_Types.h"
#include "USART_interface.h"
#include "ChipUSARTHandler_interface.h"
#include "ChipUSARTHandler_config.h"
#include "GPIO_Interface.h"
#include "RCC_interface.h"

#define NUMBER_OF_UARTS 3

extern ChipUSARTHandler_configType ChipUSARTHandler_config[NUMBER_OF_CHIPS];
static const u32 RCCPrephirals[NUMBER_OF_UARTS]={RCC_u32_CONTROL_PERIP_USART1EN,RCC_u32_CONTROL_PERIP_USART2EN,RCC_u32_CONTROL_PERIP_USART3EN};
static const u16 txGPIOPin[NUMBER_OF_UARTS]={GPIO_PIN_9,GPIO_PIN_2,GPIO_PIN_10};
static const u16 rxGPIOPin[NUMBER_OF_UARTS]={GPIO_PIN_10,GPIO_PIN_3,GPIO_PIN_11};
static const void* GPIOPorts[NUMBER_OF_UARTS]={GPIO_A_PORT,GPIO_A_PORT,GPIO_B_PORT};

u8 ChipUSARTHandler_Init(void)
{
	u8 i,error,USARTID;
	GPIO_pinType tx,rx;
	void* GPIOPort;

	error=CUH_OK;
	tx.mode=GPIO_AFOUT_OD_MODE;
	tx.speed=GPIO_50M_SPEED;
	rx.mode=GPIO_INPUT_PULL_UP_MODE;
	rx.speed=GPIO_50M_SPEED;
	for(i=0;i<NUMBER_OF_CHIPS;i++)
	{
		USARTID=ChipUSARTHandler_config[i].USARTConfig.ID;
		error|=RCC_u8_ControlPrephiral(RCCPrephirals[USARTID],RCC_u8_ENABLE);
		GPIOPort=(void*)GPIOPorts[USARTID];
		tx.port=GPIOPort;
		tx.pin=txGPIOPin[USARTID];
		rx.port=GPIOPort;
		rx.pin=rxGPIOPin[USARTID];
		GPIO_configPins(&tx);
		GPIO_configPins(&rx);
		if(ChipUSARTHandler_config[i].configMode==CUH_CONFIG_MODE_DEFAULT)
		{
			USART_init(ChipUSARTHandler_config[i].USARTConfig.ID);
		}
		else
		{
			USART_config(&ChipUSARTHandler_config[i].USARTConfig);
		}
	}
	if(error)
	{
		error=CUH_NOK;
	}
	return error;
}

u8 ChipUSARTHandler_sendBacket(u8 ID,const u8* buffer,u16 length,ChipUSARTHandler_notifyType notifyFunction)
{
	if(USART_send(ChipUSARTHandler_config[ID].USARTConfig.ID,buffer,length,notifyFunction))
	{
		return CUH_NOK;
	}
	else
	{
		return CUH_OK;
	}
}

u8 ChipUSARTHandler_receiveBacket(u8 ID,u8* buffer,u16 length,ChipUSARTHandler_notifyType notifyFunction)
{
	if(USART_receive(ChipUSARTHandler_config[ID].USARTConfig.ID,buffer,length,notifyFunction))
	{
		return CUH_NOK;
	}
	else
	{
		return CUH_OK;
	}
}
