#include "STD_Types.h"
#include "USART_interface.h"
#include "ChipUSARTHandler_interface.h"
#include "ChipUSARTHandler_config.h"

const ChipUSARTHandler_configType ChipUSARTHandler_config [NUMBER_OF_CHIPS]=
{
		{
				.USARTConfig={
					.stopBit=CUH_STOP_1,
					.baudRate=CUH_BAUDRATE_9600,
					.parity=CUH_PARITY_DISABLE,
					.asyncMode=USART_DMA_ASYNC_MODE,
					.ID=CUH_CHANNEL_USART1
				},
				.configMode=CUH_CONFIG_MODE_NON_DEFAULT
		}
};
