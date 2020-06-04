#include "STD_Types.h"
#include "SwitchHandler.h"
#include "GPIO_Interface.h"
#include "SwitchHandler_config.h"

const switchConfigType switchesConfig[NUMBER_OF_SWITCHES]=
{
	{
		.swMode=SWITCH_INPUT_PULL_UP_MODE,
		.GPIOPort=GPIO_B_PORT,
		.GPIOPin=GPIO_PIN_0,
		.pressedGPIOValue=GPIO_LOW_VALUE
	},
	{
		.swMode=SWITCH_INPUT_PULL_DOWN_MODE,
		.GPIOPort=GPIO_B_PORT,
		.GPIOPin=GPIO_PIN_1,
		.pressedGPIOValue=GPIO_HIGH_VALUE
	}
};
